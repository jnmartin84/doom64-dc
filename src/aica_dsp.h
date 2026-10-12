#ifndef AICA_DSP_H
#define AICA_DSP_H

/* The AICA's effect DSP, driven from the SH4 beside KOS's stock sound driver (no firmware of its
 * own). Game-independent: aica_dsp_gen.c builds programs (plain C, also built on the host),
 * aica_dsp_kos.c loads them and sets the channels' sends.
 *
 * Shared verbatim by the author's Dreamcast ports (quake64-decomp dc/, sm64-dc src/pc/audio/);
 * the master copy and its tests (tools/dc_audio/dsp_test.c, dsp_sim.py: the generator against a
 * step-exact DSP model and each reverb's reference) are in quake64-decomp.
 *
 * The stock driver (KOS kernel/arch/dreamcast/sound/arm/aica.c) writes only a channel's
 * registers 0x00-0x18, 0x24 (pan and the direct send) and 0x28 (volume, filter) when it starts
 * one; it zeroes everything once at snd_init and never touches the channel's DSP send (0x20),
 * the DSP's program, coefficients, addresses or outputs, or the ring buffer register (0x2804).
 *
 * The instruction format and semantics are MAME's AICA DSP (src/devices/sound/aicadsp.cpp,
 * BSD-3-Clause; ref/mame-aica), consistent with the AICA manual's register map and with the
 * aicaflow driver (MIT, ref/aicaflow) on hardware: every one of the 128 steps runs once per
 * 44.1 kHz sample; a memory read or write only on an odd step; a read's data reaches MEMS
 * through IWT two steps later; TEMP, memory and output writes store the previous step's
 * accumulator; addresses into the ring buffer are MADRS + DEC, DEC counting down one per sample.
 */

#include <stdint.h>

#define AICA_DSP_STEPS 128
#define AICA_DSP_MADRS 64

typedef struct {
    uint16_t mpro[AICA_DSP_STEPS][4]; /* the four 16-bit words of each step (0x3400 + 16 n) */
    uint16_t coef[AICA_DSP_STEPS];    /* one per step, the 13-bit coefficient << 3 (0x3000 + 4 n) */
    uint16_t madrs[AICA_DSP_MADRS];   /* ring buffer offsets in 16-bit words (0x3200 + 4 n) */
    uint32_t ring_words;              /* the ring buffer: 8192, 16384, 32768 or 65536 words */
    uint8_t outputs;                  /* EFREG outputs the program writes: 1, EFREG0 to both
                                         sides; 2, EFREG0 left and EFREG1 right */
    uint8_t steps;                    /* steps in use (the rest are idle) */
} aica_dsp_image_t;

/* One step's fields (MAME's names). */
typedef struct {
    uint8_t tra, twt, twa;               /* TEMP read address, write, write address (+ DEC) */
    uint8_t xsel, ysel, ira, iwt, iwa;   /* X: TEMP (0) or INPUTS (1); Y: FRC, COEF (1), Y_REG;
                                            INPUTS: MEMS 0-0x1F, MIXS 0x20-0x2F, EXTS 0x30-0x31;
                                            IWT: MEMS[IWA] = the read issued two steps earlier */
    uint8_t table, mwt, mrd, ewt, ewa;   /* memory write, read; EFREG[EWA] += output */
    uint8_t adrl, frcl, shift, yrl, negb, zero, bsel; /* B: TEMP (0) or ACC (1); ZERO: B = 0 */
    uint8_t nofl, masa, adreb, nxadr;    /* NOFL: memory is linear 16-bit; MASA: MADRS index */
    int16_t coef;                        /* this step's coefficient, 13-bit (4096 = 1.0) */
} aica_dsp_step_t;

void aica_dsp_image_init(aica_dsp_image_t *img);
void aica_dsp_image_step(aica_dsp_image_t *img, int n, const aica_dsp_step_t *s);

/* libultra's effect (ALFx: the aux bus's delay network) as a description: taps in samples at
 * the synthesizer's rate, coefficients in libultra's Q15, the low-pass's fc as alFxNew keeps it
 * (0: none). Chorus modulation is not modelled (a modulated section plays as a fixed delay). */
typedef struct {
    int32_t input, output;
    int16_t ffcoef, fbcoef, gain;
    int16_t lp_fc;
    uint8_t chorus;
} aica_fx_section_t;

typedef struct {
    int32_t rate;   /* the synthesizer's output rate (Hz) */
    int32_t length; /* the delay line, samples at that rate */
    int sections;
    aica_fx_section_t section[8];
} aica_fx_desc_t;

/* Builds the program for libultra's alFxPull network at the DSP's 44.1 kHz: the mono input
   (MIXS0, the channels' sends) written into the ring buffer, each section an all-pass, comb or
   low-passed comb on two taps as libultra computes it, the gained outputs summed into EFREG0.
   0 if it does not fit. */
int aica_dsp_build_libultra_fx(const aica_fx_desc_t *fx, aica_dsp_image_t *img);

/* Super Mario 64's reverb (src/audio/synthesis.c, the US engine): per side a feedback comb,
   ring[n] = send[n] + gain * ring[n - window], the side's output ring[n - window] (added to the
   dry mix at unity). window in samples at rate (the N64 ran 32000 Hz), gain in Q15 (reverbGain).
   Inputs: MIXS0 feeds both sides, MIXS1 the left only, MIXS2 the right only (a channel sends to
   one; the caller picks by the voice's pan); outputs EFREG0 (left) and EFREG1 (right). */
int aica_dsp_build_stereo_comb(int32_t window, int32_t rate, int16_t gain_q15, aica_dsp_image_t *img);

/* dc/aica_dsp_kos.c (the SH4, beside KOS's stock driver, after snd_init) */

/* Loads a program in the order the aicaflow driver uses on hardware: outputs off, the DSP's
   registers cleared, the ring buffer (2 KB aligned, in sound RAM) cleared and set, coefficients,
   addresses, program, then EFREG0 out on both sides at efsdl (0-15, 3 dB steps, 15 = 0 dB).
   0 if sound RAM has no room for the ring buffer. */
int aica_dsp_load(const aica_dsp_image_t *img, int efsdl);
/* A channel's send into the DSP's input bus (MIXS0-15): level 0 (off) to 15 (0 dB), 3 dB steps */
void aica_dsp_send_bus(int channel, int bus, int level);
/* the same into MIXS0 */
void aica_dsp_send(int channel, int level);
/* the program's outputs off (the DSP keeps running; a new aica_dsp_load turns them on) */
void aica_dsp_mute(void);
/* a wet gain (0-32768 = 0-1.0) as a send level: 15 + 20 log10(gain) / 3, rounded; 0 if silent */
int aica_dsp_level(int32_t gain_q15);

#endif
