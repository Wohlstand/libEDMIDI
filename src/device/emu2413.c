/**
 * emu2413 v1.2.4
 * https://github.com/digital-sound-antiques/emu2413
 * Copyright (C) 2020 Mitsutaka Okazaki
 *
 * This source refers to the following documents. The author would like to thank all the authors who have
 * contributed to the writing of them.
 * - [YM2413 notes](http://www.smspower.org/Development/YM2413) by andete
 * - ymf262.c by Jarek Burczynski
 * - [VRC7 presets](https://siliconpr0n.org/archive/doku.php?id=vendor:yamaha:opl2#opll_vrc7_patch_format) by Nuke.YKT
 * - YMF281B presets by Chabin
 */
#include "emu2413.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef INLINE
#if defined(_MSC_VER)
#define INLINE __inline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE inline
#endif
#endif

#define _PI_ 3.14159265358979323846264338327950288

#define OPLL_TONE_NUM 3
/* clang-format off */
static uint8_t default_inst[OPLL_TONE_NUM][(16 + 3) * 8] = {{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* 0: Original */
0x71,0x61,0x1e,0x17,0xd0,0x78,0x00,0x17, /* 1: Violin */
0x13,0x41,0x1a,0x0d,0xd8,0xf7,0x23,0x13, /* 2: Guitar */
0x13,0x01,0x99,0x00,0xf2,0xd4,0x21,0x23, /* 3: Piano */
0x11,0x61,0x0e,0x07,0x8d,0x64,0x70,0x27, /* 4: Flute */
0x32,0x21,0x1e,0x06,0xe1,0x76,0x01,0x28, /* 5: Clarinet */
0x31,0x22,0x16,0x05,0xe0,0x71,0x00,0x18, /* 6: Oboe */
0x21,0x61,0x1d,0x07,0x82,0x81,0x11,0x07, /* 7: Trumpet */
0x33,0x21,0x2d,0x13,0xb0,0x70,0x00,0x07, /* 8: Organ */
0x61,0x61,0x1b,0x06,0x64,0x65,0x10,0x17, /* 9: Horn */
0x41,0x61,0x0b,0x18,0x85,0xf0,0x81,0x07, /* A: Synthesizer */
0x33,0x01,0x83,0x11,0xea,0xef,0x10,0x04, /* B: Harpsichord */
0x17,0xc1,0x24,0x07,0xf8,0xf8,0x22,0x12, /* C: Vibraphone */
0x61,0x50,0x0c,0x05,0xd2,0xf5,0x40,0x42, /* D: Synthsizer Bass */
0x01,0x01,0x55,0x03,0xe4,0x90,0x03,0x02, /* E: Acoustic Bass */
0x41,0x41,0x89,0x03,0xf1,0xe4,0xc0,0x13, /* F: Electric Guitar */
0x01,0x01,0x18,0x0f,0xdf,0xf8,0x6a,0x6d, /* R: Bass Drum (from VRC7) */
0x01,0x01,0x00,0x00,0xc8,0xd8,0xa7,0x68, /* R: High-Hat(M) / Snare Drum(C) (from VRC7) */
0x05,0x01,0x00,0x00,0xf8,0xaa,0x59,0x55, /* R: Tom-tom(M) / Top Cymbal(C) (from VRC7) */
},{
/* VRC7 presets from Nuke.YKT */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x03,0x21,0x05,0x06,0xe8,0x81,0x42,0x27,
0x13,0x41,0x14,0x0d,0xd8,0xf6,0x23,0x12,
0x11,0x11,0x08,0x08,0xfa,0xb2,0x20,0x12,
0x31,0x61,0x0c,0x07,0xa8,0x64,0x61,0x27,
0x32,0x21,0x1e,0x06,0xe1,0x76,0x01,0x28,
0x02,0x01,0x06,0x00,0xa3,0xe2,0xf4,0xf4,
0x21,0x61,0x1d,0x07,0x82,0x81,0x11,0x07,
0x23,0x21,0x22,0x17,0xa2,0x72,0x01,0x17,
0x35,0x11,0x25,0x00,0x40,0x73,0x72,0x01,
0xb5,0x01,0x0f,0x0F,0xa8,0xa5,0x51,0x02,
0x17,0xc1,0x24,0x07,0xf8,0xf8,0x22,0x12,
0x71,0x23,0x11,0x06,0x65,0x74,0x18,0x16,
0x01,0x02,0xd3,0x05,0xc9,0x95,0x03,0x02,
0x61,0x63,0x0c,0x00,0x94,0xC0,0x33,0xf6,
0x21,0x72,0x0d,0x00,0xc1,0xd5,0x56,0x06,
0x01,0x01,0x18,0x0f,0xdf,0xf8,0x6a,0x6d,
0x01,0x01,0x00,0x00,0xc8,0xd8,0xa7,0x68,
0x05,0x01,0x00,0x00,0xf8,0xaa,0x59,0x55,
},{
/* YMF281B presets by Chabin */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x62,0x21,0x1a,0x07,0xf0,0x6f,0x00,0x16,
0x00,0x10,0x44,0x02,0xf6,0xf4,0x54,0x23,
0x03,0x01,0x97,0x04,0xf3,0xf3,0x13,0xf3,
0x01,0x61,0x0a,0x0f,0xfa,0x64,0x70,0x17,
0x22,0x21,0x1e,0x06,0xf0,0x76,0x00,0x28,
0x00,0x61,0x8a,0x0e,0xc0,0x61,0x00,0x07,
0x21,0x61,0x1b,0x07,0x84,0x80,0x17,0x17,
0x37,0x32,0xc9,0x01,0x66,0x64,0x40,0x28,
0x01,0x21,0x06,0x03,0xa5,0x71,0x51,0x07,
0x06,0x11,0x5e,0x07,0xf3,0xf2,0xf6,0x11,
0x00,0x20,0x18,0x06,0xf5,0xf3,0x20,0x26,
0x97,0x41,0x20,0x07,0xff,0xf4,0x22,0x22,
0x65,0x61,0x15,0x00,0xf7,0xf3,0x16,0xf4,
0x01,0x31,0x0e,0x07,0xfa,0xf3,0xff,0xff,
0x48,0x61,0x09,0x07,0xf1,0x94,0xf0,0xf5,
0x07,0x21,0x14,0x00,0xee,0xf8,0xff,0xf8,
0x01,0x31,0x00,0x00,0xf8,0xf7,0xf8,0xf7,
0x25,0x11,0x00,0x00,0xf8,0xfa,0xf8,0x55,
}};
/* clang-format on */

/* phase increment counter */
#define DP_BITS 19
#define DP_WIDTH (1 << DP_BITS)
#define DP_BASE_BITS (DP_BITS - PG_BITS)

/* dynamic range of envelope output */
#define EG_STEP 0.375
#define EG_BITS 7
#define EG_MUTE ((1 << EG_BITS) - 1)
#define EG_MAX (EG_MUTE - 3)

/* dynamic range of total level */
#define TL_STEP 0.75
#define TL_BITS 6

/* dynamic range of sustine level */
#define SL_STEP 3.0
#define SL_BITS 4

/* damper speed before key-on. key-scale affects. */
#define DAMPER_RATE 12

#define TL2EG(d) ((d) << (EG_BITS - TL_BITS))
#define SL2EG(d) ((d) << (EG_BITS - SL_BITS))

/* envelope phase counter size */
#define EG_DP_BITS 15

/* sine table */
#define PG_BITS 10 /* 2^10 = 1024 length sine table */
#define PG_WIDTH (1 << PG_BITS)

/* clang-format off */
/* exp_table[x] = round((exp2((double)x / 256.0) - 1) * 1024) */
static uint16_t exp_table[256] = {
0,    3,    6,    8,    11,   14,   17,   20,   22,   25,   28,   31,   34,   37,   40,   42,
45,   48,   51,   54,   57,   60,   63,   66,   69,   72,   75,   78,   81,   84,   87,   90,
93,   96,   99,   102,  105,  108,  111,  114,  117,  120,  123,  126,  130,  133,  136,  139,
142,  145,  148,  152,  155,  158,  161,  164,  168,  171,  174,  177,  181,  184,  187,  190,
194,  197,  200,  204,  207,  210,  214,  217,  220,  224,  227,  231,  234,  237,  241,  244,
248,  251,  255,  258,  262,  265,  268,  272,  276,  279,  283,  286,  290,  293,  297,  300,
304,  308,  311,  315,  318,  322,  326,  329,  333,  337,  340,  344,  348,  352,  355,  359,
363,  367,  370,  374,  378,  382,  385,  389,  393,  397,  401,  405,  409,  412,  416,  420,
424,  428,  432,  436,  440,  444,  448,  452,  456,  460,  464,  468,  472,  476,  480,  484,
488,  492,  496,  501,  505,  509,  513,  517,  521,  526,  530,  534,  538,  542,  547,  551,
555,  560,  564,  568,  572,  577,  581,  585,  590,  594,  599,  603,  607,  612,  616,  621,
625,  630,  634,  639,  643,  648,  652,  657,  661,  666,  670,  675,  680,  684,  689,  693,
698,  703,  708,  712,  717,  722,  726,  731,  736,  741,  745,  750,  755,  760,  765,  770,
774,  779,  784,  789,  794,  799,  804,  809,  814,  819,  824,  829,  834,  839,  844,  849,
854,  859,  864,  869,  874,  880,  885,  890,  895,  900,  906,  911,  916,  921,  927,  932,
937,  942,  948,  953,  959,  964,  969,  975,  980,  986,  991,  996, 1002, 1007, 1013, 1018
};
/* fullsin_table[x] = round(-log2(sin((x + 0.5) * PI / (PG_WIDTH / 4) / 2)) * 256) */
static uint16_t fullsin_table[PG_WIDTH] = {
2137, 1731, 1543, 1419, 1326, 1252, 1190, 1137, 1091, 1050, 1013, 979,  949,  920,  894,  869,
846,  825,  804,  785,  767,  749,  732,  717,  701,  687,  672,  659,  646,  633,  621,  609,
598,  587,  576,  566,  556,  546,  536,  527,  518,  509,  501,  492,  484,  476,  468,  461,
453,  446,  439,  432,  425,  418,  411,  405,  399,  392,  386,  380,  375,  369,  363,  358,
352,  347,  341,  336,  331,  326,  321,  316,  311,  307,  302,  297,  293,  289,  284,  280,
276,  271,  267,  263,  259,  255,  251,  248,  244,  240,  236,  233,  229,  226,  222,  219,
215,  212,  209,  205,  202,  199,  196,  193,  190,  187,  184,  181,  178,  175,  172,  169,
167,  164,  161,  159,  156,  153,  151,  148,  146,  143,  141,  138,  136,  134,  131,  129,
127,  125,  122,  120,  118,  116,  114,  112,  110,  108,  106,  104,  102,  100,  98,   96,
94,   92,   91,   89,   87,   85,   83,   82,   80,   78,   77,   75,   74,   72,   70,   69,
67,   66,   64,   63,   62,   60,   59,   57,   56,   55,   53,   52,   51,   49,   48,   47,
46,   45,   43,   42,   41,   40,   39,   38,   37,   36,   35,   34,   33,   32,   31,   30,
29,   28,   27,   26,   25,   24,   23,   23,   22,   21,   20,   20,   19,   18,   17,   17,
16,   15,   15,   14,   13,   13,   12,   12,   11,   10,   10,   9,    9,    8,    8,    7,
7,    7,    6,    6,    5,    5,    5,    4,    4,    4,    3,    3,    3,    2,    2,    2,
2,    1,    1,    1,    1,    1,    1,    1,    0,    0,    0,    0,    0,    0,    0,    0,
};
/* clang-format on */

static uint16_t halfsin_table[PG_WIDTH];
static uint16_t *wave_table_map[2] = {fullsin_table, halfsin_table};

/* pitch modulator */
#define PM_PG_BITS 3
#define PM_PG_WIDTH (1 << PM_PG_BITS)
#define PM_DP_BITS 22
#define PM_DP_WIDTH (1 << PM_DP_BITS)

/* offset to fnum, rough approximation of 14 cents depth. */
static int8_t pm_table[8][PM_PG_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 0},    /* fnum = 000xxxxx */
    {0, 0, 1, 0, 0, 0, -1, 0},   /* fnum = 001xxxxx */
    {0, 1, 2, 1, 0, -1, -2, -1}, /* fnum = 010xxxxx */
    {0, 1, 3, 1, 0, -1, -3, -1}, /* fnum = 011xxxxx */
    {0, 2, 4, 2, 0, -2, -4, -2}, /* fnum = 100xxxxx */
    {0, 2, 5, 2, 0, -2, -5, -2}, /* fnum = 101xxxxx */
    {0, 3, 6, 3, 0, -3, -6, -3}, /* fnum = 110xxxxx */
    {0, 3, 7, 3, 0, -3, -7, -3}, /* fnum = 111xxxxx */
};

/* amplitude lfo table */
/* The following envelop pattern is verified on real YM2413. */
/* each element repeates 64 cycles */
static uint8_t am_table[210] = {0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
                                2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,
                                4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,
                                6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,
                                8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,
                                10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
                                12, 12, 12, 12, 12, 12, 12, 12,
                                13, 13, 13,
                                12, 12, 12, 12, 12, 12, 12, 12,
                                11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10,
                                9,  9,  9,  9,  9,  9,  9,  9,  8,  8,  8,  8,  8,  8,  8,  8,
                                7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,
                                5,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,
                                3,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,
                                1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0};

/* envelope decay increment step table */
/* based on andete's research */
static uint8_t eg_step_tables[4][8] = {
    {0, 1, 0, 1, 0, 1, 0, 1},
    {0, 1, 0, 1, 1, 1, 0, 1},
    {0, 1, 1, 1, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1},
};

static uint32_t ml_table[16] = {1,     1 * 2, 2 * 2,  3 * 2,  4 * 2,  5 * 2,  6 * 2,  7 * 2,
                                8 * 2, 9 * 2, 10 * 2, 10 * 2, 12 * 2, 12 * 2, 15 * 2, 15 * 2};

#define dB2(x) ((x)*2)
static double kl_table[16] = {dB2(0.000),  dB2(9.000),  dB2(12.000), dB2(13.875), dB2(15.000), dB2(16.125),
                              dB2(16.875), dB2(17.625), dB2(18.000), dB2(18.750), dB2(19.125), dB2(19.500),
                              dB2(19.875), dB2(20.250), dB2(20.625), dB2(21.000)};

static uint32_t tll_table[8 * 16][1 << TL_BITS][4];
static int32_t rks_table[8 * 2][2];

static OPLL_PATCH null_patch = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static OPLL_PATCH default_patch[OPLL_TONE_NUM][(16 + 3) * 2];

#define min(i, j) (((i) < (j)) ? (i) : (j))
#define max(i, j) (((i) > (j)) ? (i) : (j))

/***************************************************

           Internal Sample Rate Converter

****************************************************/
/* Note: to disable internal rate converter, set clock/72 to output sampling rate. */

/*
 * LW is truncate length of sinc(x) calculation.
 * Lower LW is faster, higher LW results better quality.
 * LW must be a non-zero positive even number, no upper limit.
 * LW=16 or greater is recommended when upsampling.
 * LW=8 is practically okay for downsampling.
 */
#define LW 16

/* resolution of sinc(x) table. sinc(x) where 0.0<=x<1.0 corresponds to sinc_table[0...SINC_RESO-1] */
#define SINC_RESO 256
#define SINC_AMP_BITS 12

/* fast conversion: 3-point average filter is used instead of sinc(x) table. rough and fast.*/
#define USE_FAST_RATE_CONV 0

/* double hamming(double x) { return 0.54 - 0.46 * cos(2 * PI * x); } */
static double blackman(double x) { return 0.42 - 0.5 * cos(2 * _PI_ * x) + 0.08 * cos(4 * _PI_ * x); }
static double sinc(double x) { return (x == 0.0 ? 1.0 : sin(_PI_ * x) / (_PI_ * x)); }
static double windowed_sinc(double x) { return blackman(0.5 + 0.5 * x / (LW / 2)) * sinc(x); }

/* f_inp: input frequency. f_out: output frequencey, ch: number of channels */
OPLL_RateConv *OPLL_RateConv_new(double f_inp, double f_out, int ch) {
  OPLL_RateConv *conv = (OPLL_RateConv *) malloc(sizeof(OPLL_RateConv));
  int i;

  conv->ch = ch;
  conv->f_ratio = f_inp / f_out;
  conv->buf = (int16_t **) malloc(sizeof(void *) * ch);
  for (i = 0; i < ch; i++) {
    conv->buf[i] = (int16_t *) malloc(sizeof(conv->buf[0][0]) * LW);
  }

  /* create sinc_table for positive 0 <= x < LW/2 */
  conv->sinc_table = (int16_t *) malloc(sizeof(conv->sinc_table[0]) * SINC_RESO * LW / 2);
  for (i = 0; i < SINC_RESO * LW / 2; i++) {
    const double x = (double)i / SINC_RESO;
    if (f_out < f_inp) {
      /* for downsampling */
      conv->sinc_table[i] = (int16_t)((1 << SINC_AMP_BITS) * windowed_sinc(x / conv->f_ratio) / conv->f_ratio);
    } else {
      /* for upsampling */
      conv->sinc_table[i] = (int16_t)((1 << SINC_AMP_BITS) * windowed_sinc(x));
    }
  }

  return conv;
}

static INLINE int16_t lookup_sinc_table(int16_t *table, double x) {
  int16_t index = (int16_t)(x * SINC_RESO);
  if (index < 0)
    index = -index;
  return table[min(SINC_RESO * LW / 2 - 1, index)];
}

void OPLL_RateConv_reset(OPLL_RateConv *conv) {
  int i;
  conv->timer = 0;
  for (i = 0; i < conv->ch; i++) {
    memset(conv->buf[i], 0, sizeof(conv->buf[i][0]) * LW);
  }
}

/* put original data to this converter at f_inp. */
void OPLL_RateConv_putData(OPLL_RateConv *conv, int ch, int16_t data) {
  int16_t *buf = conv->buf[ch];
  int i;
  for (i = 0; i < LW - 1; i++) {
    buf[i] = buf[i + 1];
  }
  buf[LW - 1] = data;
}

/* get resampled data from this converter at f_out. */
/* this function must be called f_out / f_inp times per one putData call. */
int16_t OPLL_RateConv_getData(OPLL_RateConv *conv, int ch) {
  int16_t *buf = conv->buf[ch];
  int32_t sum = 0;
  int k;
  double dn;
  conv->timer += conv->f_ratio;
  dn = conv->timer - floor(conv->timer);
  conv->timer = dn;

  for (k = 0; k < LW; k++) {
    double x = ((double)k - (LW / 2 - 1)) - dn;
    sum += buf[k] * lookup_sinc_table(conv->sinc_table, x);
  }
  return sum >> SINC_AMP_BITS;
}

void OPLL_RateConv_delete(OPLL_RateConv *conv) {
  int i;
  for (i = 0; i < conv->ch; i++) {
    free(conv->buf[i]);
  }
  free(conv->buf);
  free(conv->sinc_table);
  free(conv);
}

/***************************************************

                  Create tables

****************************************************/

static void makeSinTable(void) {
  int x;
  /* for (x = 0; x < PG_WIDTH / 4; x++) {
     fullsin_table[x] = (uint16_t)round(-log2(sin((x + 0.5) * PI / (PG_WIDTH / 4) / 2)) * 256);
     } */

  for (x = 0; x < PG_WIDTH / 4; x++) {
    fullsin_table[PG_WIDTH / 4 + x] = fullsin_table[PG_WIDTH / 4 - x - 1];
  }

  for (x = 0; x < PG_WIDTH / 2; x++) {
    fullsin_table[PG_WIDTH / 2 + x] = 0x8000 | fullsin_table[x];
  }

  for (x = 0; x < PG_WIDTH / 2; x++)
    halfsin_table[x] = fullsin_table[x];

  for (x = PG_WIDTH / 2; x < PG_WIDTH; x++)
    halfsin_table[x] = 0xfff;
}

static void makeTllTable(void) {

  int32_t tmp;
  int32_t fnum, block, TL, KL;

  for (fnum = 0; fnum < 16; fnum++) {
    for (block = 0; block < 8; block++) {
      for (TL = 0; TL < 64; TL++) {
        for (KL = 0; KL < 4; KL++) {
          if (KL == 0) {
            tll_table[(block << 4) | fnum][TL][KL] = TL2EG(TL);
          } else {
            tmp = (int32_t)(kl_table[fnum] - dB2(3.000) * (7 - block));
            if (tmp <= 0)
              tll_table[(block << 4) | fnum][TL][KL] = TL2EG(TL);
            else
              tll_table[(block << 4) | fnum][TL][KL] = (uint32_t)((tmp >> (3 - KL)) / EG_STEP) + TL2EG(TL);
          }
        }
      }
    }
  }
}

static void makeRksTable(void) {
  int fnum8, block;
  for (fnum8 = 0; fnum8 < 2; fnum8++)
    for (block = 0; block < 8; block++) {
      rks_table[(block << 1) | fnum8][1] = (block << 1) + fnum8;
      rks_table[(block << 1) | fnum8][0] = block >> 1;
    }
}

static void makeDefaultPatch() {
  int i, j;
  for (i = 0; i < OPLL_TONE_NUM; i++)
    for (j = 0; j < 19; j++)
      OPLL_getDefaultPatch(i, j, &default_patch[i][j * 2]);
}

static uint8_t table_initialized = 0;

static void initializeTables() {
  makeTllTable();
  makeRksTable();
  makeSinTable();
  makeDefaultPatch();
  table_initialized = 1;
}

/*********************************************************

                      Synthesizing

*********************************************************/
#define SLOT_BD1 12
#define SLOT_BD2 13
#define SLOT_HH 14
#define SLOT_SD 15
#define SLOT_TOM 16
#define SLOT_CYM 17

/* utility macros */
#define MOD(o, x) (&(o)->slot[(x) << 1])
#define CAR(o, x) (&(o)->slot[((x) << 1) | 1])
#define BIT(s, b) (((s) >> (b)) & 1)

#define OPLL_DEBUG 0

#if OPLL_DEBUG
static void _debug_print_patch(OPLL_SLOT *slot) {
  OPLL_PATCH *p = slot->patch;
  printf("[slot#%d am:%d pm:%d eg:%d kr:%d ml:%d kl:%d tl:%d wf:%d fb:%d A:%d D:%d S:%d R:%d]\n", slot->number, /**/
         p->AM, p->PM, p->EG, p->KR, p->ML,                                                                     /**/
         p->KL, p->TL, p->WF, p->FB,                                                                            /**/
         p->AR, p->DR, p->SL, p->RR);
}

static char *_debug_eg_state_name(OPLL_SLOT *slot) {
  switch (slot->eg_state) {
  case ATTACK:
    return "attack";
  case DECAY:
    return "decay";
  case SUSTAIN:
    return "sustain";
  case RELEASE:
    return "release";
  case DAMP:
    return "damp";
  default:
    return "unknown";
  }
}

static INLINE void _debug_print_slot_info(OPLL_SLOT *slot) {
  char *name = _debug_eg_state_name(slot);
  printf("[slot#%d state:%s fnum:%03x rate:%d-%d]\n", slot->number, name, slot->blk_fnum, slot->eg_rate_h,
         slot->eg_rate_l);
  _debug_print_patch(slot);
  fflush(stdout);
}
#endif

static INLINE int get_parameter_rate(OPLL_SLOT *slot) {
  switch (slot->eg_state) {
  case ATTACK:
    return slot->patch->AR;
  case DECAY:
    return slot->patch->DR;
  case SUSTAIN:
    return slot->patch->EG ? 0 : slot->patch->RR;
  case RELEASE:
    if (slot->sus_flag) {
      return 5;
    } else if (slot->patch->EG) {
      return slot->patch->RR;
    } else {
      return 7;
    }
  case DAMP:
    return DAMPER_RATE;
  default:
    return 0;
  }
}

enum SLOT_UPDATE_FLAG {
  UPDATE_WF = 1,
  UPDATE_TLL = 2,
  UPDATE_RKS = 4,
  UPDATE_EG = 8,
  UPDATE_ALL = 255
};

static INLINE void request_update(OPLL_SLOT *slot, int flag) { slot->update_requests |= flag; }

static void commit_slot_update(OPLL_SLOT *slot) {

#if OPLL_DEBUG
  if (slot->last_eg_state != slot->eg_state) {
    _debug_print_slot_info(slot);
    slot->last_eg_state = slot->eg_state;
  }
#endif

  if (slot->update_requests & UPDATE_WF) {
    slot->wave_table = wave_table_map[slot->patch->WF];
  }

  if (slot->update_requests & UPDATE_TLL) {
    if ((slot->type & 1) == 0) {
      slot->tll = tll_table[slot->blk_fnum >> 5][slot->patch->TL][slot->patch->KL];
    } else {
      slot->tll = tll_table[slot->blk_fnum >> 5][slot->volume][slot->patch->KL];
    }
  }

  if (slot->update_requests & UPDATE_RKS) {
    slot->rks = rks_table[slot->blk_fnum >> 8][slot->patch->KR];
  }

  if (slot->update_requests & (UPDATE_RKS | UPDATE_EG)) {
    int p_rate = get_parameter_rate(slot);

    if (p_rate == 0) {
      slot->eg_shift = 0;
      slot->eg_rate_h = 0;
      slot->eg_rate_l = 0;
      return;
    }

    slot->eg_rate_h = min(15, p_rate + (slot->rks >> 2));
    slot->eg_rate_l = slot->rks & 3;
    if (slot->eg_state == ATTACK) {
      slot->eg_shift = (0 < slot->eg_rate_h && slot->eg_rate_h < 12) ? (13 - slot->eg_rate_h) : 0;
    } else {
      slot->eg_shift = (slot->eg_rate_h < 13) ? (13 - slot->eg_rate_h) : 0;
    }
  }

  slot->update_requests = 0;
}

static void reset_slot(OPLL_SLOT *slot, int number) {
  slot->number = number;
  slot->type = number % 2;
  slot->pg_keep = 0;
  slot->wave_table = wave_table_map[0];
  slot->pg_phase = 0;
  slot->output[0] = 0;
  slot->output[1] = 0;
  slot->eg_state = RELEASE;
  slot->eg_shift = 0;
  slot->rks = 0;
  slot->tll = 0;
  slot->sus_flag = 0;
  slot->blk_fnum = 0;
  slot->blk = 0;
  slot->fnum = 0;
  slot->volume = 0;
  slot->pg_out = 0;
  slot->eg_out = EG_MUTE;
  slot->patch = &null_patch;
}

static INLINE void slotOn(OPLL *opll, int i) {
  OPLL_SLOT *slot = &opll->slot[i];
  slot->eg_state = DAMP;
  request_update(slot, UPDATE_EG);
}

static INLINE void slotOff(OPLL *opll, int i) {
  OPLL_SLOT *slot = &opll->slot[i];
  if (slot->type & 1) {
    slot->eg_state = RELEASE;
    request_update(slot, UPDATE_EG);
  }
}

static INLINE void update_key_status(OPLL *opll) {
  const uint8_t r14 = opll->reg[0x0e];
  const uint8_t rhythm_mode = BIT(r14, 5);
  uint32_t new_slot_key_status = 0;
  uint32_t updated_status;
  int ch;

  for (ch = 0; ch < 9; ch++)
    if (opll->reg[0x20 + ch] & 0x10)
      new_slot_key_status |= 3 << (ch * 2);

  if (rhythm_mode) {
    if (r14 & 0x10)
      new_slot_key_status |= 3 << SLOT_BD1;

    if (r14 & 0x01)
      new_slot_key_status |= 1 << SLOT_HH;

    if (r14 & 0x08)
      new_slot_key_status |= 1 << SLOT_SD;

    if (r14 & 0x04)
      new_slot_key_status |= 1 << SLOT_TOM;

    if (r14 & 0x02)
      new_slot_key_status |= 1 << SLOT_CYM;
  }

  updated_status = opll->slot_key_status ^ new_slot_key_status;

  if (updated_status) {
    int i;
    for (i = 0; i < 18; i++)
      if (BIT(updated_status, i)) {
        if (BIT(new_slot_key_status, i)) {
          slotOn(opll, i);
        } else {
          slotOff(opll, i);
        }
      }
  }

  opll->slot_key_status = new_slot_key_status;
}

static INLINE void set_patch(OPLL *opll, int32_t ch, int32_t num) {
  opll->patch_number[ch] = num;
  MOD(opll, ch)->patch = &opll->patch[num * 2 + 0];
  CAR(opll, ch)->patch = &opll->patch[num * 2 + 1];
  request_update(MOD(opll, ch), UPDATE_ALL);
  request_update(CAR(opll, ch), UPDATE_ALL);
}

static INLINE void set_slot_patch(OPLL_SLOT *slot, OPLL_PATCH *patch) {
  slot->patch = patch;
  request_update(slot, UPDATE_ALL);
}

static INLINE void set_sus_flag(OPLL *opll, int ch, int flag) {
  CAR(opll, ch)->sus_flag = flag;
  request_update(CAR(opll, ch), UPDATE_EG);
  if (MOD(opll, ch)->type & 1) {
    MOD(opll, ch)->sus_flag = flag;
    request_update(MOD(opll, ch), UPDATE_EG);
  }
}

/* set volume ( volume : 6bit, register value << 2 ) */
static INLINE void set_volume(OPLL *opll, int ch, int volume) {
  CAR(opll, ch)->volume = volume;
  request_update(CAR(opll, ch), UPDATE_TLL);
}

static INLINE void set_slot_volume(OPLL_SLOT *slot, int volume) {
  slot->volume = volume;
  request_update(slot, UPDATE_TLL);
}

/* set f-Nnmber ( fnum : 9bit ) */
static INLINE void set_fnumber(OPLL *opll, int ch, int fnum) {
  OPLL_SLOT *car = CAR(opll, ch);
  OPLL_SLOT *mod = MOD(opll, ch);
  car->fnum = fnum;
  car->blk_fnum = (car->blk_fnum & 0xe00) | (fnum & 0x1ff);
  mod->fnum = fnum;
  mod->blk_fnum = (mod->blk_fnum & 0xe00) | (fnum & 0x1ff);
  request_update(car, UPDATE_EG | UPDATE_RKS | UPDATE_TLL);
  request_update(mod, UPDATE_EG | UPDATE_RKS | UPDATE_TLL);
}

/* set block data (blk : 3bit ) */
static INLINE void set_block(OPLL *opll, int ch, int blk) {
  OPLL_SLOT *car = CAR(opll, ch);
  OPLL_SLOT *mod = MOD(opll, ch);
  car->blk = blk;
  car->blk_fnum = ((blk & 7) << 9) | (car->blk_fnum & 0x1ff);
  mod->blk = blk;
  mod->blk_fnum = ((blk & 7) << 9) | (mod->blk_fnum & 0x1ff);
  request_update(car, UPDATE_EG | UPDATE_RKS | UPDATE_TLL);
  request_update(mod, UPDATE_EG | UPDATE_RKS | UPDATE_TLL);
}

static INLINE void update_rhythm_mode(OPLL *opll) {
  const uint8_t new_rhythm_mode = (opll->reg[0x0e] >> 5) & 1;
  const uint32_t slot_key_status = opll->slot_key_status;

  if (opll->patch_number[6] & 0x10) {
    if (!(BIT(slot_key_status, SLOT_BD2) | new_rhythm_mode)) {
      opll->slot[SLOT_BD1].eg_state = RELEASE;
      opll->slot[SLOT_BD1].eg_out = EG_MUTE;
      opll->slot[SLOT_BD2].eg_state = RELEASE;
      opll->slot[SLOT_BD2].eg_out = EG_MUTE;
      set_patch(opll, 6, opll->reg[0x36] >> 4);
    }
  } else if (new_rhythm_mode) {
    opll->patch_number[6] = 16;
    opll->slot[SLOT_BD1].eg_state = RELEASE;
    opll->slot[SLOT_BD1].eg_out = EG_MUTE;
    opll->slot[SLOT_BD2].eg_state = RELEASE;
    opll->slot[SLOT_BD2].eg_out = EG_MUTE;
    set_slot_patch(&opll->slot[SLOT_BD1], &opll->patch[16 * 2 + 0]);
    set_slot_patch(&opll->slot[SLOT_BD2], &opll->patch[16 * 2 + 1]);
  }

  if (opll->patch_number[7] & 0x10) {
    if (!((BIT(slot_key_status, SLOT_HH) && BIT(slot_key_status, SLOT_SD)) | new_rhythm_mode)) {
      opll->slot[SLOT_HH].type = 0;
      opll->slot[SLOT_HH].pg_keep = 0;
      opll->slot[SLOT_HH].eg_state = RELEASE;
      opll->slot[SLOT_HH].eg_out = EG_MUTE;
      opll->slot[SLOT_SD].type = 1;
      opll->slot[SLOT_SD].eg_state = RELEASE;
      opll->slot[SLOT_SD].eg_out = EG_MUTE;
      set_patch(opll, 7, opll->reg[0x37] >> 4);
    }
  } else if (new_rhythm_mode) {
    opll->patch_number[7] = 17;
    opll->slot[SLOT_HH].type = 3;
    opll->slot[SLOT_HH].pg_keep = 1;
    opll->slot[SLOT_HH].eg_state = RELEASE;
    opll->slot[SLOT_HH].eg_out = EG_MUTE;
    opll->slot[SLOT_SD].type = 3;
    opll->slot[SLOT_SD].eg_state = RELEASE;
    opll->slot[SLOT_SD].eg_out = EG_MUTE;
    set_slot_patch(&opll->slot[SLOT_HH], &opll->patch[17 * 2 + 0]);
    set_slot_patch(&opll->slot[SLOT_SD], &opll->patch[17 * 2 + 1]);
    set_slot_volume(&opll->slot[SLOT_HH], ((opll->reg[0x37] >> 4) & 15) << 2);
  }

  if (opll->patch_number[8] & 0x10) {
    if (!((BIT(slot_key_status, SLOT_CYM) && BIT(slot_key_status, SLOT_TOM)) | new_rhythm_mode)) {
      opll->slot[SLOT_TOM].type = 0;
      opll->slot[SLOT_TOM].eg_state = RELEASE;
      opll->slot[SLOT_TOM].eg_out = EG_MUTE;
      opll->slot[SLOT_CYM].type = 1;
      opll->slot[SLOT_CYM].pg_keep = 0;
      opll->slot[SLOT_CYM].eg_state = RELEASE;
      opll->slot[SLOT_CYM].eg_out = EG_MUTE;
      set_patch(opll, 8, opll->reg[0x38] >> 4);
    }
  } else if (new_rhythm_mode) {
    opll->patch_number[8] = 18;
    opll->slot[SLOT_TOM].type = 3;
    opll->slot[SLOT_TOM].eg_state = RELEASE;
    opll->slot[SLOT_TOM].eg_out = EG_MUTE;
    opll->slot[SLOT_CYM].type = 3;
    opll->slot[SLOT_CYM].pg_keep = 1;
    opll->slot[SLOT_CYM].eg_state = RELEASE;
    opll->slot[SLOT_CYM].eg_out = EG_MUTE;
    set_slot_patch(&opll->slot[SLOT_TOM], &opll->patch[18 * 2 + 0]);
    set_slot_patch(&opll->slot[SLOT_CYM], &opll->patch[18 * 2 + 1]);
    set_slot_volume(&opll->slot[SLOT_TOM], ((opll->reg[0x38] >> 4) & 15) << 2);
  }

  opll->rhythm_mode = new_rhythm_mode;
}

static void update_ampm(OPLL *opll) {
  const uint32_t pm_inc = (opll->test_flag & 8) ? opll->pm_dphase << 10 : opll->pm_dphase;
  const uint32_t am_inc = (opll->test_flag & 8) ? 64 : 1;
  if (opll->test_flag & 2) {
    opll->pm_phase = 0;
    opll->am_phase = 0;
  } else {
    opll->pm_phase = (opll->pm_phase + pm_inc) & (PM_DP_WIDTH - 1);
    opll->am_phase += am_inc;
  }
  opll->lfo_am = am_table[(opll->am_phase >> 6) % sizeof(am_table)];
}

static void update_noise(OPLL *opll) {
  if (opll->noise_seed & 1)
    opll->noise_seed ^= 0x8003020;
  opll->noise_seed >>= 1;
  opll->noise = opll->noise_seed & 1;
}

static void update_short_noise(OPLL *opll) {
  const uint32_t pg_hh = opll->slot[SLOT_HH].pg_out;
  const uint32_t pg_cym = opll->slot[SLOT_CYM].pg_out;

  const uint8_t h_bit2 = BIT(pg_hh, PG_BITS - 8);
  const uint8_t h_bit7 = BIT(pg_hh, PG_BITS - 3);
  const uint8_t h_bit3 = BIT(pg_hh, PG_BITS - 7);

  const uint8_t c_bit3 = BIT(pg_cym, PG_BITS - 7);
  const uint8_t c_bit5 = BIT(pg_cym, PG_BITS - 5);

  opll->short_noise = (h_bit2 ^ h_bit7) | (h_bit3 ^ c_bit5) | (c_bit3 ^ c_bit5);
}

static INLINE void calc_phase(OPLL_SLOT *slot, int32_t pm_phase, uint8_t test) {
  const int8_t pm = slot->patch->PM ? pm_table[(slot->fnum >> 6) & 7][pm_phase >> (PM_DP_BITS - PM_PG_BITS)] : 0;
  if (test) {
    slot->pg_phase = (((slot->fnum & 0x1ff) * 2 + pm) * ml_table[slot->patch->ML]) << slot->blk >> 2;
  } else {
    slot->pg_phase += (((slot->fnum & 0x1ff) * 2 + pm) * ml_table[slot->patch->ML]) << slot->blk >> 2;
    slot->pg_phase &= (DP_WIDTH - 1);
  }
  slot->pg_out = slot->pg_phase >> DP_BASE_BITS;
}

static INLINE uint8_t lookup_attack_step(OPLL_SLOT *slot, uint32_t counter) {
  int index;

  switch (slot->eg_rate_h) {
  case 12:
    index = (counter & 0xc) >> 1;
    return 4 - eg_step_tables[slot->eg_rate_l][index];
  case 13:
    index = (counter & 0xc) >> 1;
    return 3 - eg_step_tables[slot->eg_rate_l][index];
  case 14:
    index = (counter & 0xc) >> 1;
    return 2 - eg_step_tables[slot->eg_rate_l][index];
  case 0:
  case 15:
    return 0;
  default:
    index = counter >> slot->eg_shift;
    return eg_step_tables[slot->eg_rate_l][index & 7] ? 4 : 0;
  }
}

static INLINE uint8_t lookup_decay_step(OPLL_SLOT *slot, uint32_t counter) {
  int index;

  switch (slot->eg_rate_h) {
  case 0:
    return 0;
  case 13:
    index = ((counter & 0xc) >> 1) | (counter & 1);
    return eg_step_tables[slot->eg_rate_l][index];
  case 14:
    index = ((counter & 0xc) >> 1);
    return eg_step_tables[slot->eg_rate_l][index] + 1;
  case 15:
    return 2;
  default:
    index = counter >> slot->eg_shift;
    return eg_step_tables[slot->eg_rate_l][index & 7];
  }
}

static INLINE void finish_damp_state(OPLL_SLOT *slot) {
  if (min(15, slot->patch->AR + (slot->rks >> 2)) == 15) {
    slot->eg_state = DECAY;
    slot->eg_out = 0;
  } else {
    slot->eg_state = ATTACK;
    slot->eg_out = EG_MUTE;
  }
  slot->pg_phase = slot->pg_keep ? slot->pg_phase : 0;
  request_update(slot, UPDATE_EG);
}

static INLINE void calc_envelope(OPLL_SLOT *slot, OPLL_SLOT *slave_slot, uint16_t eg_counter, uint8_t test) {

  uint32_t mask = (1 << slot->eg_shift) - 1;
  uint8_t s;

  if (slot->eg_state == ATTACK) {
    if (0 < slot->eg_out && slot->eg_rate_h > 0 && (eg_counter & mask & ~3) == 0) {
      s = lookup_attack_step(slot, eg_counter);
      if (0 < s) {
        slot->eg_out = max(0, ((int)slot->eg_out - (slot->eg_out >> s) - 1));
      }
    }
  } else {
    if (slot->eg_rate_h > 0 && (eg_counter & mask) == 0) {
      slot->eg_out = min(EG_MUTE, slot->eg_out + lookup_decay_step(slot, eg_counter));
    }
  }

  switch (slot->eg_state) {
  case DAMP:
    if (slot->eg_out >= EG_MAX) {
      if ((slot->type & 1) && (!slave_slot || slave_slot->eg_out >= EG_MAX)) {
        finish_damp_state(slot);
        if (slave_slot) {
          finish_damp_state(slave_slot);
        }
      }
    }
    break;

  case ATTACK:
    if (slot->eg_out == 0) {
      slot->eg_state = DECAY;
      request_update(slot, UPDATE_EG);
    }
    break;

  case DECAY:
    if ((slot->eg_out >> (EG_BITS - SL_BITS)) == slot->patch->SL) {
      slot->eg_state = SUSTAIN;
      request_update(slot, UPDATE_EG);
    }
    break;

  case SUSTAIN:
  case RELEASE:
  default:
    break;
  }

  if (test) {
    slot->eg_out = 0;
  }
}

static void update_slots(OPLL *opll) {
  int i;
  opll->eg_counter++;

  for (i = 0; i < 18; i++) {
    OPLL_SLOT *slot = &opll->slot[i];
    OPLL_SLOT *slave;
    if (slot->update_requests) {
      commit_slot_update(slot);
    }
    calc_phase(slot, opll->pm_phase, opll->test_flag & 4);

    slave = slot->type == 1 ? &opll->slot[i - 1] : NULL;
    calc_envelope(slot, slave, opll->eg_counter, opll->test_flag & 1);
  }
}

/* output: -4095...4095 */
static INLINE int16_t lookup_exp_table(uint16_t i) {
  /* from andete's expressoin */
  int16_t t = (exp_table[(i & 0xff) ^ 0xff] + 1024);
  int16_t res = t >> ((i & 0x7f00) >> 8);
  return ((i & 0x8000) ? ~res : res) << 1;
}

static INLINE int16_t to_linear(uint16_t h, OPLL_SLOT *slot, int16_t am) {
  uint16_t att;
  if (slot->eg_out >= EG_MAX)
    return 0;

  att = min(127, (slot->eg_out + slot->tll + am)) << 4;
  return lookup_exp_table(h + att);
}

static INLINE int16_t calc_slot_car(OPLL *opll, int ch, int16_t fm) {
  OPLL_SLOT *slot = CAR(opll, ch);

  uint8_t am = slot->patch->AM ? opll->lfo_am : 0;

  return to_linear(slot->wave_table[(slot->pg_out + 2 * fm) & (PG_WIDTH - 1)], slot, am);
}

static INLINE int16_t calc_slot_mod(OPLL *opll, int ch) {
  OPLL_SLOT *slot = MOD(opll, ch);

  int16_t fm = slot->patch->FB > 0 ? (slot->output[1] + slot->output[0]) >> (8 - slot->patch->FB) : 0;
  uint8_t am = slot->patch->AM ? opll->lfo_am : 0;

  slot->output[1] = slot->output[0];
  slot->output[0] = to_linear(slot->wave_table[(slot->pg_out + fm) & (PG_WIDTH - 1)], slot, am) >> 1;

  return slot->output[0];
}

static INLINE int16_t calc_slot_tom(OPLL *opll) {
  OPLL_SLOT *slot = MOD(opll, 8);

  return to_linear(slot->wave_table[slot->pg_out], slot, 0);
}

/* Specify phase offset directly based on 10-bit (1024-length) sine table */
#define _PD(phase) ((PG_BITS < 10) ? (phase >> (10 - PG_BITS)) : (phase << (PG_BITS - 10)))

static INLINE int16_t calc_slot_snare(OPLL *opll) {
  OPLL_SLOT *slot = CAR(opll, 7);

  uint32_t phase;

  if (BIT(slot->pg_out, PG_BITS - 2))
    phase = opll->noise ? _PD(0x300) : _PD(0x200);
  else
    phase = opll->noise ? _PD(0x0) : _PD(0x100);

  return to_linear(slot->wave_table[phase], slot, 0);
}

static INLINE int16_t calc_slot_cym(OPLL *opll) {
  OPLL_SLOT *slot = CAR(opll, 8);

  uint32_t phase = opll->short_noise ? _PD(0x300) : _PD(0x100);

  return to_linear(slot->wave_table[phase], slot, 0);
}

static INLINE int16_t calc_slot_hat(OPLL *opll) {
  OPLL_SLOT *slot = MOD(opll, 7);

  uint32_t phase;

  if (opll->short_noise)
    phase = opll->noise ? _PD(0x2d0) : _PD(0x234);
  else
    phase = opll->noise ? _PD(0x34) : _PD(0xd0);

  return to_linear(slot->wave_table[phase], slot, 0);
}

#define _MO(x) (-(x) >> 1)
#define _RO(x) (x)

static void update_output(OPLL *opll) {
  int16_t *out;
  int i;

  update_ampm(opll);
  update_noise(opll);
  update_short_noise(opll);
  update_slots(opll);

  out = opll->ch_out;

  /* CH1-6 */
  for (i = 0; i < 6; i++) {
    if (!(opll->mask & OPLL_MASK_CH(i))) {
      out[i] = _MO(calc_slot_car(opll, i, calc_slot_mod(opll, i)));
    }
  }

  /* CH7 */
  if (opll->patch_number[6] <= 15) {
    if (!(opll->mask & OPLL_MASK_CH(6))) {
      out[6] = _MO(calc_slot_car(opll, 6, calc_slot_mod(opll, 6)));
    }
  } else {
    if (!(opll->mask & OPLL_MASK_BD)) {
      out[9] = _RO(calc_slot_car(opll, 6, calc_slot_mod(opll, 6)));
    }
  }

  /* CH8 */
  if (opll->patch_number[7] <= 15) {
    if (!(opll->mask & OPLL_MASK_CH(7))) {
      out[7] = _MO(calc_slot_car(opll, 7, calc_slot_mod(opll, 7)));
    }
  } else {
    if (!(opll->mask & OPLL_MASK_HH)) {
      out[10] = _RO(calc_slot_hat(opll));
    }
    if (!(opll->mask & OPLL_MASK_SD)) {
      out[11] = _RO(calc_slot_snare(opll));
    }
  }

  /* CH9 */
  if (opll->patch_number[8] <= 15) {
    if (!(opll->mask & OPLL_MASK_CH(8))) {
      out[8] = _MO(calc_slot_car(opll, 8, calc_slot_mod(opll, 8)));
    }
  } else {
    if (!(opll->mask & OPLL_MASK_TOM)) {
      out[12] = _RO(calc_slot_tom(opll));
    }
    if (!(opll->mask & OPLL_MASK_CYM)) {
      out[13] = _RO(calc_slot_cym(opll));
    }
  }
}

INLINE static void mix_output(OPLL *opll) {
  int16_t out = 0;
  int i;
  for (i = 0; i < 15; i++) {
    out += opll->ch_out[i];
  }
  if (opll->conv) {
    OPLL_RateConv_putData(opll->conv, 0, out);
  } else {
    opll->mix_out[0] = out;
  }
}

INLINE static void mix_output_stereo(OPLL *opll) {
  int16_t *out = opll->mix_out;
  int i;
  out[0] = out[1] = 0;
  for (i = 0; i < 15; i++) {
    if (opll->pan[i] & 1)
      out[1] += opll->ch_out[i];
    if (opll->pan[i] & 2)
      out[0] += opll->ch_out[i];
  }
  if (opll->conv) {
    OPLL_RateConv_putData(opll->conv, 0, out[0]);
    OPLL_RateConv_putData(opll->conv, 1, out[1]);
  }
}

/***********************************************************

                   External Interfaces

***********************************************************/

OPLL *OPLL_new(uint32_t clk, uint32_t rate) {
  OPLL *opll;
  int i;

  if (!table_initialized) {
    initializeTables();
  }

  opll = (OPLL *)calloc(sizeof(OPLL), 1);
  if (opll == NULL)
    return NULL;

  for (i = 0; i < 19 * 2; i++)
    memcpy(&opll->patch[i], &null_patch, sizeof(OPLL_PATCH));

  opll->clk = clk;
  opll->rate = rate;
  opll->mask = 0;
  opll->conv = NULL;
  opll->mix_out[0] = 0;
  opll->mix_out[1] = 0;

  OPLL_reset(opll);
  OPLL_reset_patch(opll, 0);

  return opll;
}

void OPLL_delete(OPLL *opll) {
  if (opll->conv) {
    OPLL_RateConv_delete(opll->conv);
    opll->conv = NULL;
  }
  free(opll);
}

static void reset_rate_conversion_params(OPLL *opll) {
  const double f_out = opll->rate;
  const double f_inp = opll->clk / 72;

  opll->out_time = 0;
  opll->out_step = ((uint32_t)f_inp) << 8;
  opll->inp_step = ((uint32_t)f_out) << 8;

  if (opll->conv) {
    OPLL_RateConv_delete(opll->conv);
    opll->conv = NULL;
  }

  if (floor(f_inp) != f_out && floor(f_inp + 0.5) != f_out) {
    opll->conv = OPLL_RateConv_new(f_inp, f_out, 2);
  }

  if (opll->conv) {
    OPLL_RateConv_reset(opll->conv);
  }
}

void OPLL_reset(OPLL *opll) {
  int i;

  if (!opll)
    return;

  opll->adr = 0;

  opll->pm_phase = 0;
  opll->am_phase = 0;

  opll->noise_seed = 0xffff;
  opll->mask = 0;

  opll->rhythm_mode = 0;
  opll->slot_key_status = 0;
  opll->eg_counter = 0;

  reset_rate_conversion_params(opll);

  for (i = 0; i < 18; i++)
    reset_slot(&opll->slot[i], i);

  for (i = 0; i < 9; i++) {
    set_patch(opll, i, 0);
  }

  for (i = 0; i < 0x40; i++)
    OPLL_writeReg(opll, i, 0);

  opll->pm_dphase = PM_DP_WIDTH / (1024 * 8);

  for (i = 0; i < 15; i++)
    opll->pan[i] = 3;

  for (i = 0; i < 15; i++) {
    opll->ch_out[i] = 0;
  }
}

void OPLL_forceRefresh(OPLL *opll) {
  int i;

  if (opll == NULL)
    return;

  for (i = 0; i < 9; i++) {
    set_patch(opll, i, opll->patch_number[i]);
  }

  for (i = 0; i < 18; i++) {
    request_update(&opll->slot[i], UPDATE_ALL);
  }
}

void OPLL_setRate(OPLL *opll, uint32_t rate) {
  opll->rate = rate;
  reset_rate_conversion_params(opll);
}

void OPLL_setQuality(OPLL *opll, uint8_t q) {(void)opll; (void)q;}

void OPLL_setChipMode(OPLL *opll, uint8_t mode) { opll->chip_mode = mode; }

void OPLL_writeReg(OPLL *opll, uint32_t reg, uint8_t data) {
  int32_t ch;
  int i;

  data = data & 0xff;
  reg = reg & 0x3f;

  /* mirror registers */
  if ((0x19 <= reg && reg <= 0x1f) || (0x29 <= reg && reg <= 0x2f) || (0x39 <= reg && reg <= 0x3f)) {
    reg -= 9;
  }

  opll->reg[reg] = (uint8_t)data;

  switch (reg) {
  case 0x00:
    opll->patch[0].AM = (data >> 7) & 1;
    opll->patch[0].PM = (data >> 6) & 1;
    opll->patch[0].EG = (data >> 5) & 1;
    opll->patch[0].KR = (data >> 4) & 1;
    opll->patch[0].ML = (data)&15;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(MOD(opll, i), UPDATE_RKS | UPDATE_EG);
      }
    }
    break;

  case 0x01:
    opll->patch[1].AM = (data >> 7) & 1;
    opll->patch[1].PM = (data >> 6) & 1;
    opll->patch[1].EG = (data >> 5) & 1;
    opll->patch[1].KR = (data >> 4) & 1;
    opll->patch[1].ML = (data)&15;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(CAR(opll, i), UPDATE_RKS | UPDATE_EG);
      }
    }
    break;

  case 0x02:
    opll->patch[0].KL = (data >> 6) & 3;
    opll->patch[0].TL = (data)&63;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(MOD(opll, i), UPDATE_TLL);
      }
    }
    break;

  case 0x03:
    opll->patch[1].KL = (data >> 6) & 3;
    opll->patch[1].WF = (data >> 4) & 1;
    opll->patch[0].WF = (data >> 3) & 1;
    opll->patch[0].FB = (data)&7;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(MOD(opll, i), UPDATE_WF);
        request_update(CAR(opll, i), UPDATE_WF | UPDATE_TLL);
      }
    }
    break;

  case 0x04:
    opll->patch[0].AR = (data >> 4) & 15;
    opll->patch[0].DR = (data)&15;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(MOD(opll, i), UPDATE_EG);
      }
    }
    break;

  case 0x05:
    opll->patch[1].AR = (data >> 4) & 15;
    opll->patch[1].DR = (data)&15;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(CAR(opll, i), UPDATE_EG);
      }
    }
    break;

  case 0x06:
    opll->patch[0].SL = (data >> 4) & 15;
    opll->patch[0].RR = (data)&15;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(MOD(opll, i), UPDATE_EG);
      }
    }
    break;

  case 0x07:
    opll->patch[1].SL = (data >> 4) & 15;
    opll->patch[1].RR = (data)&15;
    for (i = 0; i < 9; i++) {
      if (opll->patch_number[i] == 0) {
        request_update(CAR(opll, i), UPDATE_EG);
      }
    }
    break;

  case 0x0e:
    if (opll->chip_mode == 1)
      break;
    update_rhythm_mode(opll);
    update_key_status(opll);
    break;

  case 0x0f:
    opll->test_flag = data;
    break;

  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
  case 0x16:
  case 0x17:
  case 0x18:
    ch = reg - 0x10;
    set_fnumber(opll, ch, data + ((opll->reg[0x20 + ch] & 1) << 8));
    break;

  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
  case 0x25:
  case 0x26:
  case 0x27:
  case 0x28:
    ch = reg - 0x20;
    set_fnumber(opll, ch, ((data & 1) << 8) + opll->reg[0x10 + ch]);
    set_block(opll, ch, (data >> 1) & 7);
    set_sus_flag(opll, ch, (data >> 5) & 1);
    update_key_status(opll);
    /* update rhythm mode here because key-off of rhythm instrument is deferred until key-on bit is down. */
    update_rhythm_mode(opll);
    break;

  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x36:
  case 0x37:
  case 0x38:
    if ((opll->reg[0x0e] & 32) && (reg >= 0x36)) {
      switch (reg) {
      case 0x37:
        set_slot_volume(MOD(opll, 7), ((data >> 4) & 15) << 2);
        break;
      case 0x38:
        set_slot_volume(MOD(opll, 8), ((data >> 4) & 15) << 2);
        break;
      default:
        break;
      }
    } else {
      set_patch(opll, reg - 0x30, (data >> 4) & 15);
    }
    set_volume(opll, reg - 0x30, (data & 15) << 2);
    break;

  default:
    break;
  }
}

void OPLL_writeIO(OPLL *opll, uint32_t adr, uint8_t val) {
  if (adr & 1)
    OPLL_writeReg(opll, opll->adr, val);
  else
    opll->adr = val;
}

void OPLL_setPan(OPLL *opll, uint32_t ch, uint8_t pan) { opll->pan[ch & 15] = pan & 3; }

void OPLL_dumpToPatch(const uint8_t *dump, OPLL_PATCH *patch) {
  patch[0].AM = (dump[0] >> 7) & 1;
  patch[1].AM = (dump[1] >> 7) & 1;
  patch[0].PM = (dump[0] >> 6) & 1;
  patch[1].PM = (dump[1] >> 6) & 1;
  patch[0].EG = (dump[0] >> 5) & 1;
  patch[1].EG = (dump[1] >> 5) & 1;
  patch[0].KR = (dump[0] >> 4) & 1;
  patch[1].KR = (dump[1] >> 4) & 1;
  patch[0].ML = (dump[0]) & 15;
  patch[1].ML = (dump[1]) & 15;
  patch[0].KL = (dump[2] >> 6) & 3;
  patch[1].KL = (dump[3] >> 6) & 3;
  patch[0].TL = (dump[2]) & 63;
  patch[1].TL = 0;
  patch[0].FB = (dump[3]) & 7;
  patch[1].FB = 0;
  patch[0].WF = (dump[3] >> 3) & 1;
  patch[1].WF = (dump[3] >> 4) & 1;
  patch[0].AR = (dump[4] >> 4) & 15;
  patch[1].AR = (dump[5] >> 4) & 15;
  patch[0].DR = (dump[4]) & 15;
  patch[1].DR = (dump[5]) & 15;
  patch[0].SL = (dump[6] >> 4) & 15;
  patch[1].SL = (dump[7] >> 4) & 15;
  patch[0].RR = (dump[6]) & 15;
  patch[1].RR = (dump[7]) & 15;
}

void OPLL_getDefaultPatch(int32_t type, int32_t num, OPLL_PATCH *patch) {
  OPLL_dump2patch(default_inst[type] + num * 8, patch);
}

void OPLL_setPatch(OPLL *opll, const uint8_t *dump) {
  OPLL_PATCH patch[2];
  int i;
  for (i = 0; i < 19; i++) {
    OPLL_dump2patch(dump + i * 8, patch);
    memcpy(&opll->patch[i * 2 + 0], &patch[0], sizeof(OPLL_PATCH));
    memcpy(&opll->patch[i * 2 + 1], &patch[1], sizeof(OPLL_PATCH));
  }
}

void OPLL_patchToDump(const OPLL_PATCH *patch, uint8_t *dump) {
  dump[0] = (uint8_t)((patch[0].AM << 7) + (patch[0].PM << 6) + (patch[0].EG << 5) + (patch[0].KR << 4) + patch[0].ML);
  dump[1] = (uint8_t)((patch[1].AM << 7) + (patch[1].PM << 6) + (patch[1].EG << 5) + (patch[1].KR << 4) + patch[1].ML);
  dump[2] = (uint8_t)((patch[0].KL << 6) + patch[0].TL);
  dump[3] = (uint8_t)((patch[1].KL << 6) + (patch[1].WF << 4) + (patch[0].WF << 3) + patch[0].FB);
  dump[4] = (uint8_t)((patch[0].AR << 4) + patch[0].DR);
  dump[5] = (uint8_t)((patch[1].AR << 4) + patch[1].DR);
  dump[6] = (uint8_t)((patch[0].SL << 4) + patch[0].RR);
  dump[7] = (uint8_t)((patch[1].SL << 4) + patch[1].RR);
}

void OPLL_copyPatch(OPLL *opll, int32_t num, OPLL_PATCH *patch) {
  memcpy(&opll->patch[num], patch, sizeof(OPLL_PATCH));
}

void OPLL_resetPatch(OPLL *opll, int32_t type) {
  int i;
  for (i = 0; i < 19 * 2; i++)
    OPLL_copyPatch(opll, i, &default_patch[type % OPLL_TONE_NUM][i]);
}

int16_t OPLL_calc(OPLL *opll) {
  while (opll->out_step > opll->out_time) {
    opll->out_time += opll->inp_step;
    update_output(opll);
    mix_output(opll);
  }
  opll->out_time -= opll->out_step;
  if (opll->conv) {
    opll->mix_out[0] = OPLL_RateConv_getData(opll->conv, 0);
  }
  return opll->mix_out[0];
}

void OPLL_calcStereo(OPLL *opll, int32_t out[2]) {
  while (opll->out_step > opll->out_time) {
    opll->out_time += opll->inp_step;
    update_output(opll);
    mix_output_stereo(opll);
  }
  opll->out_time -= opll->out_step;
  if (opll->conv) {
    out[0] = OPLL_RateConv_getData(opll->conv, 0);
    out[1] = OPLL_RateConv_getData(opll->conv, 1);
  } else {
    out[0] = opll->mix_out[0];
    out[1] = opll->mix_out[1];
  }
}

uint32_t OPLL_setMask(OPLL *opll, uint32_t mask) {
  uint32_t ret;

  if (opll) {
    ret = opll->mask;
    opll->mask = mask;
    return ret;
  } else
    return 0;
}

uint32_t OPLL_toggleMask(OPLL *opll, uint32_t mask) {
  uint32_t ret;

  if (opll) {
    ret = opll->mask;
    opll->mask ^= mask;
    return ret;
  } else
    return 0;
}
