/* The AICA DSP on the SH4, beside KOS's stock sound driver: loading a program built by
 * dc/aica_dsp_gen.c and setting the channels' sends. The registers are written over the G2 bus
 * (g2_write_32 waits for its FIFO). Register offsets from the AICA's base (0xA0700000 for the
 * SH4): a channel's at 0x80 * n, its DSP send at + 0x20 (ISEL bits 3-0, IMXL bits 7-4); the
 * outputs' levels and pans at 0x2000 + 4 n (EFSDL bits 11-8, EFPAN bits 4-0); the ring buffer at
 * 0x2804 (RBL bits 14-13, RBP bits 11-0 in 2 KB units); COEF at 0x3000, MADRS at 0x3200, MPRO at
 * 0x3400 (four 16-bit words per step, 4 bytes apart), TEMP at 0x4000, MEMS at 0x4400, EFREG at
 * 0x4580. */
#ifdef _arch_dreamcast /* KOS builds only (a port may compile this directory for other targets) */

#include <stdint.h>
#include <stdio.h>

#include <dc/g2bus.h>
#include <dc/spu.h>
#include <dc/sound/sound.h>

#include "aica_dsp.h"

#define AICA_REGS 0xA0700000u

static uint32_t sRing;      /* the ring buffer in sound RAM (2 KB aligned) */
static uint32_t sRingRaw;   /* its allocation */
static uint32_t sRingBytes;

static void reg(uint32_t offset, uint32_t value) {
    g2_write_32(AICA_REGS + offset, value);
}

int aica_dsp_load(const aica_dsp_image_t *img, int efsdl) {
    uint32_t bytes = img->ring_words * 2, rbl, i, w;

    /* outputs off, then everything the DSP keeps cleared */
    reg(0x2000, 0);
    reg(0x2004, 0);
    for (i = 0; i < 0x300; i++) reg(0x3000 + i * 4, 0); /* COEF, MADRS, MPRO */
    for (i = 0; i < 256; i++) reg(0x4000 + i * 4, 0);   /* TEMP */
    for (i = 0; i < 64; i++) reg(0x4400 + i * 4, 0);    /* MEMS */
    for (i = 0; i < 16; i++) reg(0x4580 + i * 4, 0);    /* EFREG */

    if (sRing == 0 || sRingBytes < bytes) {
        /* a larger ring than the last program's (a game may load several): replace it */
        if (sRingRaw != 0) snd_mem_free(sRingRaw);
        sRingRaw = snd_mem_malloc(bytes + 2048);
        if (sRingRaw == 0) {
            printf("AICA DSP: no sound RAM for a %u-byte ring buffer\n", (unsigned) bytes);
            sRing = 0;
            sRingBytes = 0;
            return 0;
        }
        sRing = (sRingRaw + 2047) & ~2047u;
        sRingBytes = bytes;
    }
    spu_memset(sRing, 0, bytes); /* the program uses linear 16-bit memory (NOFL): 0 is silence */
    for (rbl = 0; (8192u << rbl) < img->ring_words; rbl++) {
    }
    reg(0x2804, (rbl << 13) | (sRing >> 11));

    for (i = 0; i < AICA_DSP_STEPS; i++) reg(0x3000 + i * 4, img->coef[i]);
    for (i = 0; i < AICA_DSP_MADRS; i++) reg(0x3200 + i * 4, img->madrs[i]);
    for (i = 0; i < AICA_DSP_STEPS; i++) {
        for (w = 0; w < 4; w++) reg(0x3400 + i * 16 + w * 4, img->mpro[i][w]);
    }
    /* EFPAN 0: left and right at 0 dB; 0x1F: left only; 0x0F: right only */
    if (img->outputs == 1) {
        reg(0x2000, ((uint32_t) (efsdl & 0xF) << 8) | 0x00); /* EFREG0 to both sides */
    } else if (img->outputs == 2) {
        reg(0x2000, ((uint32_t) (efsdl & 0xF) << 8) | 0x1F); /* EFREG0 left */
        reg(0x2004, ((uint32_t) (efsdl & 0xF) << 8) | 0x0F); /* EFREG1 right */
    }
    printf("AICA DSP: %d steps, %u-word ring buffer at %X\n", img->steps, (unsigned) img->ring_words,
           (unsigned) sRing);
    return 1;
}

void aica_dsp_send_bus(int channel, int bus, int level) {
    reg((uint32_t) channel * 0x80 + 0x20, ((uint32_t) (level & 0xF) << 4) | ((uint32_t) bus & 0xF));
}

void aica_dsp_send(int channel, int level) {
    aica_dsp_send_bus(channel, 0, level);
}

void aica_dsp_mute(void) {
    reg(0x2000, 0);
    reg(0x2004, 0);
}

#endif
