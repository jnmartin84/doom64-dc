/* AICA DSP programs, built on the SH4 (or on the host for tests): the instruction encoder and
 * libultra's effect network (alFxPull, src/libultra/audio/reverb.c). Plain C, no KOS. See
 * dc/aica_dsp.h for the instruction semantics this relies on and where they come from. */
#include <math.h>
#include <string.h>

#include "aica_dsp.h"

#define DSP_RATE 44100
#define UNITY 4095 /* the largest 13-bit coefficient: 1.0 - 1/4096 */
#define MIXS0 0x20 /* INPUTS index of MIXS[0], where every channel sends (ISEL 0) */

void aica_dsp_image_init(aica_dsp_image_t *img) {
    aica_dsp_step_t idle;
    int n;

    memset(img, 0, sizeof(*img));
    /* an idle step: ACC = X * COEF (0) + 0, nothing written (aicaflow's NOP) */
    memset(&idle, 0, sizeof(idle));
    idle.ysel = 1;
    idle.zero = 1;
    for (n = 0; n < AICA_DSP_STEPS; n++) {
        aica_dsp_image_step(img, n, &idle);
    }
    img->ring_words = 8192;
}

void aica_dsp_image_step(aica_dsp_image_t *img, int n, const aica_dsp_step_t *s) {
    img->mpro[n][0] = (uint16_t) ((s->tra & 0x7F) << 9 | (s->twt & 1) << 8 | (s->twa & 0x7F) << 1);
    img->mpro[n][1] = (uint16_t) ((s->xsel & 1) << 15 | (s->ysel & 3) << 13 | (s->ira & 0x3F) << 7 |
                                  (s->iwt & 1) << 6 | (s->iwa & 0x1F) << 1);
    img->mpro[n][2] = (uint16_t) ((s->table & 1) << 15 | (s->mwt & 1) << 14 | (s->mrd & 1) << 13 |
                                  (s->ewt & 1) << 12 | (s->ewa & 0xF) << 8 | (s->adrl & 1) << 7 |
                                  (s->frcl & 1) << 6 | (s->shift & 3) << 4 | (s->yrl & 1) << 3 |
                                  (s->negb & 1) << 2 | (s->zero & 1) << 1 | (s->bsel & 1));
    img->mpro[n][3] = (uint16_t) ((s->nofl & 1) << 15 | (s->masa & 0x1F) << 9 | (s->adreb & 1) << 8 |
                                  (s->nxadr & 1) << 7);
    img->coef[n] = (uint16_t) ((uint16_t) s->coef << 3);
}

/* ------------------------------------------------------------------------------------------
 * The emitter: steps in pairs. The even step computes (ACC = X * COEF + B, B the accumulator or
 * zero); the odd step keeps ACC as it is (ACC = X * 0 + ACC) and stores it (TEMP, memory,
 * EFREG), or reads memory. So a value is computed in an even step and stored in the odd step
 * after it, and a sum may run over several pairs.
 * ---------------------------------------------------------------------------------------- */

typedef struct {
    uint8_t temp; /* 1: TEMP[slot]; 0: MEMS[slot] (through INPUTS) */
    uint8_t slot;
} src_t;

typedef struct {
    aica_dsp_image_t *img;
    int pair;
    int nmadrs;
    int ntemp;
    int npersist;
    int overflow;
} gen_t;

#define TEMP_PERSIST 64 /* persistent TEMP pairs from here (written at L, read next sample at L+1) */

static int masa_for(gen_t *g, uint32_t offset) {
    int i;
    for (i = 0; i < g->nmadrs; i++) {
        if (g->img->madrs[i] == offset) return i;
    }
    if (g->nmadrs == AICA_DSP_MADRS || g->nmadrs == 32) { /* MASA is 5 bits */
        g->overflow = 1;
        return 0;
    }
    g->img->madrs[g->nmadrs] = (uint16_t) offset;
    return g->nmadrs++;
}

static void emit(gen_t *g, const aica_dsp_step_t *even, const aica_dsp_step_t *odd) {
    aica_dsp_step_t e, o;
    if (2 * g->pair + 1 >= AICA_DSP_STEPS) {
        g->overflow = 1;
        return;
    }
    if (even != NULL) {
        e = *even;
    } else { /* keep ACC */
        memset(&e, 0, sizeof(e));
        e.ysel = 1;
        e.bsel = 1;
    }
    if (odd != NULL) {
        o = *odd;
    } else {
        memset(&o, 0, sizeof(o));
    }
    /* the odd step keeps ACC whatever it stores or reads */
    o.ysel = 1;
    o.coef = 0;
    o.bsel = 1;
    o.zero = 0;
    o.xsel = 0;
    aica_dsp_image_step(g->img, 2 * g->pair, &e);
    aica_dsp_image_step(g->img, 2 * g->pair + 1, &o);
    g->pair++;
}

/* ACC = src * coef (+ ACC unless first) */
static aica_dsp_step_t term(src_t src, int coef, int first) {
    aica_dsp_step_t s;
    memset(&s, 0, sizeof(s));
    if (src.temp) {
        s.xsel = 0;
        s.tra = src.slot;
    } else {
        s.xsel = 1;
        s.ira = src.slot;
    }
    s.ysel = 1;
    s.coef = (int16_t) coef;
    s.zero = first ? 1 : 0;
    s.bsel = first ? 0 : 1;
    return s;
}

static src_t temp_src(int slot) {
    src_t s = { 1, (uint8_t) slot };
    return s;
}

static int new_temp(gen_t *g) {
    if (g->ntemp == TEMP_PERSIST - 1) g->overflow = 1;
    return g->ntemp++;
}

int aica_dsp_level(int32_t gain_q15) {
    int lvl;
    if (gain_q15 <= 0) return 0;
    if (gain_q15 >= 32768) return 15;
    lvl = 15 + (int) floor(20.0 * log10((double) gain_q15 / 32768.0) / 3.0 + 0.5);
    return lvl < 0 ? 0 : lvl;
}

/* libultra's Q15 coefficient as the DSP's 13-bit one */
static int q15(int c) {
    int v = (c >= 0) ? (c + 4) / 8 : -((-c + 4) / 8);
    if (v > UNITY) v = UNITY;
    if (v < -4096) v = -4096;
    return v;
}

/* ------------------------------------------------------------------------------------------
 * libultra's network
 * ---------------------------------------------------------------------------------------- */

#define MAX_POS 32

typedef struct {
    uint32_t off[MAX_POS];
    src_t val[MAX_POS];
    int n;
} posmap_t;

static int pos_find(const posmap_t *m, uint32_t off) {
    int i;
    for (i = 0; i < m->n; i++) {
        if (m->off[i] == off) return i;
    }
    return -1;
}

static void pos_set(posmap_t *m, uint32_t off, src_t v) {
    int i = pos_find(m, off);
    if (i < 0) {
        if (m->n == MAX_POS) return;
        i = m->n++;
        m->off[i] = off;
    }
    m->val[i] = v;
}

int aica_dsp_build_libultra_fx(const aica_fx_desc_t *fx, aica_dsp_image_t *img) {
    gen_t g;
    posmap_t written; /* ring positions written earlier in this sample, and where the value is */
    uint32_t reads[MAX_POS];
    int nreads = 0;
    src_t out_src[8];
    int out_gain[8];
    int nout = 0;
    uint32_t ring, longest;
    int i, k, x;
    aica_dsp_step_t e, o;
    double ratio = (double) DSP_RATE / (double) fx->rate;

#define OFF(samples) ((uint32_t) ((double) (samples) * ratio + 0.5))

    aica_dsp_image_init(img);
    memset(&g, 0, sizeof(g));
    g.img = img;
    if (fx->sections <= 0 || fx->sections > 8) return 0;

    longest = OFF(fx->length);
    for (i = 0; i < fx->sections; i++) {
        if (OFF(fx->section[i].output) > longest) longest = OFF(fx->section[i].output);
        if (OFF(fx->section[i].input) > longest) longest = OFF(fx->section[i].input);
    }
    for (ring = 8192; ring < longest + 1 && ring < 65536; ring <<= 1) {
    }
    if (longest + 1 > ring) return 0;
    img->ring_words = ring;

    /* Planning: which taps are read before this sample writes them (from memory, read first,
       before any write) and which reuse a value this sample has already written. Position 0
       holds the input. */
    memset(&written, 0, sizeof(written));
    pos_set(&written, 0, temp_src(0));
    for (i = 0; i < fx->sections; i++) {
        const aica_fx_section_t *s = &fx->section[i];
        uint32_t in = OFF(s->input), out = OFF(s->output);
        uint32_t taps[2] = { in, out };
        for (k = 0; k < 2; k++) {
            int j, seen = 0;
            if (pos_find(&written, taps[k]) >= 0) continue;
            for (j = 0; j < nreads; j++) seen |= reads[j] == taps[k];
            if (!seen && nreads < 32) reads[nreads++] = taps[k];
        }
        if (s->fbcoef) pos_set(&written, in, temp_src(0));
        if (!s->chorus) pos_set(&written, out, temp_src(0));
    }

    /* the input: x = MIXS0, written to the ring at position 0 */
    memset(&written, 0, sizeof(written));
    x = new_temp(&g);
    {
        src_t mix = { 0, MIXS0 };
        e = term(mix, UNITY, 1);
    }
    memset(&o, 0, sizeof(o));
    o.twt = 1;
    o.twa = (uint8_t) x;
    o.mwt = 1;
    o.nofl = 1;
    o.masa = (uint8_t) masa_for(&g, 0);
    emit(&g, &e, &o);
    pos_set(&written, 0, temp_src(x));

    /* the memory reads: one per odd step, each landing in MEMS two steps later */
    for (i = 0; i <= nreads; i++) {
        memset(&o, 0, sizeof(o));
        if (i < nreads) {
            o.mrd = 1;
            o.nofl = 1;
            o.masa = (uint8_t) masa_for(&g, reads[i]);
        }
        if (i > 0) {
            o.iwt = 1;
            o.iwa = (uint8_t) (i - 1);
        }
        emit(&g, NULL, &o);
    }

    /* the sections, in libultra's order */
    for (i = 0; i < fx->sections; i++) {
        const aica_fx_section_t *s = &fx->section[i];
        uint32_t in = OFF(s->input), out = OFF(s->output);
        src_t a, b, y;
        int p, j, lp = s->lp_fc != 0, write_out = !s->chorus;

        p = pos_find(&written, in);
        if (p >= 0) {
            a = written.val[p];
        } else {
            for (j = 0; j < nreads && reads[j] != in; j++) {
            }
            a.temp = 0;
            a.slot = (uint8_t) j;
        }
        p = pos_find(&written, out);
        if (p >= 0) {
            b = written.val[p];
        } else {
            for (j = 0; j < nreads && reads[j] != out; j++) {
            }
            b.temp = 0;
            b.slot = (uint8_t) j;
        }

        /* y = d[out] + ffcoef * d[in] (the section's output, before the low-pass) */
        if (s->ffcoef || !b.temp) {
            int t = new_temp(&g);
            e = term(b, UNITY, 1);
            if (s->ffcoef) {
                emit(&g, &e, NULL);
                e = term(a, q15(s->ffcoef), 0);
            }
            memset(&o, 0, sizeof(o));
            o.twt = 1;
            o.twa = (uint8_t) t;
            /* the output tap is written back only if its value changed (the low-pass writes it
               itself; without a feed-forward y is d[out] as read) */
            if (write_out && !lp && s->ffcoef) {
                o.mwt = 1;
                o.nofl = 1;
                o.masa = (uint8_t) masa_for(&g, out);
            }
            emit(&g, &e, &o);
            y = temp_src(t);
        } else {
            y = b; /* already this sample's value, nothing to write back */
        }

        /* d[in] = d[in] + fbcoef * y */
        if (s->fbcoef) {
            int t = new_temp(&g);
            e = term(a, UNITY, 1);
            emit(&g, &e, NULL);
            e = term(y, q15(s->fbcoef), 0);
            memset(&o, 0, sizeof(o));
            o.twt = 1;
            o.twa = (uint8_t) t;
            o.mwt = 1;
            o.nofl = 1;
            o.masa = (uint8_t) masa_for(&g, in);
            emit(&g, &e, &o);
            pos_set(&written, in, temp_src(t));
        }

        /* the one-pole low-pass (libultra's POLEF: y = (1 - c) x + c y', c = fc / 2 / 16384),
           its coefficient taken to the DSP's rate; its state persists in TEMP across samples */
        if (lp) {
            double c = (double) ((s->lp_fc * 16384) >> 15) / 16384.0, cd;
            int ca, cb, L = TEMP_PERSIST + 2 * g.npersist++;
            if (c < 0) c = 0;
            cd = pow(c, (double) fx->rate / (double) DSP_RATE);
            ca = (int) (cd * 4096.0 + 0.5);
            if (ca > UNITY) ca = UNITY;
            cb = 4096 - ca;
            if (cb > UNITY) cb = UNITY;
            if (L + 1 >= AICA_DSP_STEPS) g.overflow = 1;
            e = term(temp_src(L + 1), ca, 1);
            emit(&g, &e, NULL);
            e = term(y, cb, 0);
            memset(&o, 0, sizeof(o));
            o.twt = 1;
            o.twa = (uint8_t) L;
            if (write_out) {
                o.mwt = 1;
                o.nofl = 1;
                o.masa = (uint8_t) masa_for(&g, out);
            }
            emit(&g, &e, &o);
            y = temp_src(L);
        }
        if (write_out) pos_set(&written, out, y);
        if (s->gain && nout < 8) {
            out_src[nout] = y;
            out_gain[nout] = q15(s->gain);
            nout++;
        }
    }

    /* the output: the sections' gained outputs, summed into EFREG0 */
    for (i = 0; i < nout; i++) {
        e = term(out_src[i], out_gain[i], i == 0);
        memset(&o, 0, sizeof(o));
        if (i == nout - 1) {
            o.ewt = 1;
            o.ewa = 0;
        }
        emit(&g, &e, (i == nout - 1) ? &o : NULL);
    }
    img->outputs = nout ? 1 : 0;
    img->steps = (uint8_t) (2 * g.pair);
    return !g.overflow;
#undef OFF
}

/* ------------------------------------------------------------------------------------------
 * Super Mario 64's reverb: a feedback comb per side
 * ---------------------------------------------------------------------------------------- */

int aica_dsp_build_stereo_comb(int32_t window, int32_t rate, int16_t gain_q15, aica_dsp_image_t *img) {
    gen_t g;
    uint32_t d, ring, half;
    int side, gain = q15(gain_q15);
    aica_dsp_step_t e, o;
    src_t delayed[2] = { { 0, 0 }, { 0, 1 } }; /* MEMS0: the left line's tap, MEMS1: the right's */

    aica_dsp_image_init(img);
    memset(&g, 0, sizeof(g));
    g.img = img;
    if (window <= 0 || rate <= 0) return 0;
    d = (uint32_t) ((double) window * DSP_RATE / rate + 0.5);
    /* two lines in one ring, half a ring apart: the left written at 0 and read at d, the right
       at half and half + d */
    for (ring = 8192; ring < 2 * (d + 1) && ring < 65536; ring <<= 1) {
    }
    if (2 * (d + 1) > ring) return 0;
    img->ring_words = ring;
    half = ring / 2;

    /* the two taps: one read per odd step, landing in MEMS two steps later */
    memset(&o, 0, sizeof(o));
    o.mrd = 1;
    o.nofl = 1;
    o.masa = (uint8_t) masa_for(&g, d);
    emit(&g, NULL, &o);
    memset(&o, 0, sizeof(o));
    o.mrd = 1;
    o.nofl = 1;
    o.masa = (uint8_t) masa_for(&g, half + d);
    o.iwt = 1;
    o.iwa = 0;
    emit(&g, NULL, &o);
    memset(&o, 0, sizeof(o));
    o.iwt = 1;
    o.iwa = 1;
    emit(&g, NULL, &o);

    for (side = 0; side < 2; side++) {
        src_t both = { 0, MIXS0 }, own = { 0, (uint8_t) (MIXS0 + 1 + side) };
        /* the side's output: the delayed line at unity */
        e = term(delayed[side], UNITY, 1);
        memset(&o, 0, sizeof(o));
        o.ewt = 1;
        o.ewa = (uint8_t) side;
        emit(&g, &e, &o);
        /* the line: send (both sides' bus + this side's bus) + gain * delayed */
        e = term(delayed[side], gain, 1);
        emit(&g, &e, NULL);
        e = term(both, UNITY, 0);
        emit(&g, &e, NULL);
        e = term(own, UNITY, 0);
        memset(&o, 0, sizeof(o));
        o.mwt = 1;
        o.nofl = 1;
        o.masa = (uint8_t) masa_for(&g, side ? half : 0);
        emit(&g, &e, &o);
    }
    img->outputs = 2;
    img->steps = (uint8_t) (2 * g.pair);
    return !g.overflow;
}
