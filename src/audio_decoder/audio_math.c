/*****************************************************************************
 * audio_math.c: Inverse Discrete Cosine Transform and Pulse Code Modulation
 *****************************************************************************
 * Copyright (C) 1999, 2000 VideoLAN
 *
 * Authors:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include <stdio.h>                                           /* "intf_msg.h" */
#include <stdlib.h>                                      /* malloc(), free() */
#include <sys/types.h>                        /* on BSD, uio.h needs types.h */
#include <sys/uio.h>                                            /* "input.h" */

#include "config.h"
#include "common.h"
#include "mtime.h"                                                /* mtime_t */
#include "threads.h"

#include "intf_msg.h"                        /* intf_DbgMsg(), intf_ErrMsg() */

#include "input.h"                      /* pes_packet_t (for decoder_fifo.h) */
#include "decoder_fifo.h"            /* decoder_fifo_t (for audio_decoder.h) */

#include "audio_output.h"               /* aout_fifo_t (for audio_decoder.h) */

#include "audio_decoder.h"                                    /* adec_bank_t */

/*****************************************************************************
 * DCT32: Fast 32 points Discrete Cosine Transform
 *****************************************************************************
 * 289 additions and multiplications
 * F(u)=alpha(u)*SUM(x=0, x<N) f(x)*cos((2x+1)u*pi/2N)
 * where alpha(u) = sqrt(2)/N if u=0, 2/N otherwise.
 * See fastdct.ps, and fast.tar.gz for a (Fortran :) implementation.
 *****************************************************************************/

void DCT32(float *x, adec_bank_t *b)
{
    /* cosine coefficients */
    static const float c2  =  .70710678118655;
    static const float c3  =  .54119610014620;
    static const float c4  = -1.3065629648764;
    static const float c5  =  .50979557910416;
    static const float c6  =  .89997622313642;
    static const float c7  = -2.5629154477415;
    static const float c8  = -.60134488693505;
    static const float c9  =  .50241928618816;
    static const float c10 =  .56694403481636;
    static const float c11 =  .78815462345125;
    static const float c12 =  1.7224470982383;
    static const float c13 = -5.1011486186892;
    static const float c14 = -1.0606776859903;
    static const float c15 = -.64682178335999;
    static const float c16 = -.52249861493969;
    static const float c17 =  .50060299823520;
    static const float c18 =  .51544730992262;
    static const float c19 =  .55310389603444;
    static const float c20 =  .62250412303566;
    static const float c21 =  .74453627100230;
    static const float c22 =  .97256823786196;
    static const float c23 =  1.4841646163142;
    static const float c24 =  3.4076084184687;
    static const float c25 = -10.190008123548;
    static const float c26 = -2.0577810099534;
    static const float c27 = -1.1694399334329;
    static const float c28 = -.83934964541553;
    static const float c29 = -.67480834145501;
    static const float c30 = -.58293496820613;
    static const float c31 = -.53104259108978;
    static const float c32 = -.50547095989754;

    /* temporary variables */
    float  t1  , t2  , t3  , t4  , t5  , t6  , t7  , t8  ,
           t9  , t10 , t11 , t12 , t13 , t14 , t15 , t16 ,
           t17 , t18 , t19 , t20 , t21 , t22 , t23 , t24 ,
           t25 , t26 , t27 , t28 , t29 , t30 , t31 , t32 ,
           tt1 , tt2 , tt3 , tt4 , tt5 , tt6 , tt7 , tt8 ,
           tt9 , tt10, tt11, tt12, tt13, tt14, tt15, tt16,
           tt17, tt18, tt19, tt20, tt21, tt22, tt23, tt24,
           tt25, tt26, tt27, tt28, tt29, tt30, tt31, tt32, *y;

    /* We unrolled the loops */
    /* Odd-even ordering is integrated before the 1st stage */
    t17 = c17 * (x[0] - x[31]);
    t1  = x[0] + x[31];
    t18 = c18 * (x[2] - x[29]);
    t2  = x[2] + x[29];
    t19 = c19 * (x[4] - x[27]);
    t3  = x[4] + x[27];
    t20 = c20 * (x[6] - x[25]);
    t4  = x[6] + x[25];
    t21 = c21 * (x[8] - x[23]);
    t5  = x[8] + x[23];
    t22 = c22 * (x[10] - x[21]);
    t6  = x[10] + x[21];
    t23 = c23 * (x[12] - x[19]);
    t7  = x[12] + x[19];
    t24 = c24 * (x[14] - x[17]);
    t8  = x[14] + x[17];
    t25 = c25 * (x[16] - x[15]);
    t9  = x[16] + x[15];
    t26 = c26 * (x[18] - x[13]);
    t10 = x[18] + x[13];
    t27 = c27 * (x[20] - x[11]);
    t11 = x[20] + x[11];
    t28 = c28 * (x[22] - x[9]);
    t12 = x[22] + x[9];
    t29 = c29 * (x[24] - x[7]);
    t13 = x[24] + x[7];
    t30 = c30 * (x[26] - x[5]);
    t14 = x[26] + x[5];
    t31 = c31 * (x[28] - x[3]);
    t15 = x[28] + x[3];
    t32 = c32 * (x[30] - x[1]);
    t16 = x[30] + x[1];
    /* 2nd stage */
    tt9  = c9  * (t1  - t9 );
    tt1  = t1  + t9;
    tt10 = c10 * (t2  - t10);
    tt2  = t2  + t10;
    tt11 = c11 * (t3  - t11);
    tt3  = t3  + t11;
    tt12 = c12 * (t4  - t12);
    tt4  = t4  + t12;
    tt13 = c13 * (t5  - t13);
    tt5  = t5  + t13;
    tt14 = c14 * (t6  - t14);
    tt6  = t6  + t14;
    tt15 = c15 * (t7  - t15);
    tt7  = t7  + t15;
    tt16 = c16 * (t8  - t16);
    tt8  = t8  + t16;
    tt25 = c9  * (t17 - t25);
    tt17 = t17 + t25;
    tt26 = c10 * (t18 - t26);
    tt18 = t18 + t26;
    tt27 = c11 * (t19 - t27);
    tt19 = t19 + t27;
    tt28 = c12 * (t20 - t28);
    tt20 = t20 + t28;
    tt29 = c13 * (t21 - t29);
    tt21 = t21 + t29;
    tt30 = c14 * (t22 - t30);
    tt22 = t22 + t30;
    tt31 = c15 * (t23 - t31);
    tt23 = t23 + t31;
    tt32 = c16 * (t24 - t32);
    tt24 = t24 + t32;
    /* 3rd stage */
    t5  = c5 * (tt1  - tt5 );
    t1  = tt1  + tt5;
    t6  = c6 * (tt2  - tt6 );
    t2  = tt2  + tt6;
    t7  = c7 * (tt3  - tt7 );
    t3  = tt3  + tt7;
    t8  = c8 * (tt4  - tt8 );
    t4  = tt4  + tt8;
    t13 = c5 * (tt9  - tt13);
    t9  = tt9  + tt13;
    t14 = c6 * (tt10 - tt14);
    t10 = tt10 + tt14;
    t15 = c7 * (tt11 - tt15);
    t11 = tt11 + tt15;
    t16 = c8 * (tt12 - tt16);
    t12 = tt12 + tt16;
    t21 = c5 * (tt17 - tt21);
    t17 = tt17 + tt21;
    t22 = c6 * (tt18 - tt22);
    t18 = tt18 + tt22;
    t23 = c7 * (tt19 - tt23);
    t19 = tt19 + tt23;
    t24 = c8 * (tt20 - tt24);
    t20 = tt20 + tt24;
    t29 = c5 * (tt25 - tt29);
    t25 = tt25 + tt29;
    t30 = c6 * (tt26 - tt30);
    t26 = tt26 + tt30;
    t31 = c7 * (tt27 - tt31);
    t27 = tt27 + tt31;
    t32 = c8 * (tt28 - tt32);
    t28 = tt28 + tt32;
    /* 4th stage */
    tt3  = c3 * (t1  - t3 );
    tt1  = t1  + t3;
    tt4  = c4 * (t2  - t4 );
    tt2  = t2  + t4;
    tt7  = c3 * (t5  - t7 );
    tt5  = t5  + t7;
    tt8  = c4 * (t6  - t8 );
    tt6  = t6  + t8;
    tt11 = c3 * (t9  - t11);
    tt9  = t9  + t11;
    tt12 = c4 * (t10 - t12);
    tt10 = t10 + t12;
    tt15 = c3 * (t13 - t15);
    tt13 = t13 + t15;
    tt16 = c4 * (t14 - t16);
    tt14 = t14 + t16;
    tt19 = c3 * (t17 - t19);
    tt17 = t17 + t19;
    tt20 = c4 * (t18 - t20);
    tt18 = t18 + t20;
    tt23 = c3 * (t21 - t23);
    tt21 = t21 + t23;
    tt24 = c4 * (t22 - t24);
    tt22 = t22 + t24;
    tt27 = c3 * (t25 - t27);
    tt25 = t25 + t27;
    tt28 = c4 * (t26 - t28);
    tt26 = t26 + t28;
    tt31 = c3 * (t29 - t31);
    tt29 = t29 + t31;
    tt32 = c4 * (t30 - t32);
    tt30 = t30 + t32;
    /* Bit-reverse ordering is integrated after the 5th stage */
    /* Begin to split the result of the DCT (t1 to t32) in the filter bank */
    x = b->actual + b->pos;
    y = (b->actual == b->v1 ? b->v2 : b->v1) + b->pos;
    x[0] = -(y[0] = c2 * (tt1  - tt2 )); /* t17 */
    x[256] = 0; y[256] = tt1  + tt2; /* t1  */
    t25 = c2 * (tt3  - tt4 );
    t9  = tt3  + tt4;
    t21 = c2 * (tt5  - tt6 );
    t5  = tt5  + tt6;
    t29 = c2 * (tt7  - tt8 );
    t13 = tt7  + tt8;
    t19 = c2 * (tt9  - tt10);
    t3  = tt9  + tt10;
    t27 = c2 * (tt11 - tt12);
    t11 = tt11 + tt12;
    t23 = c2 * (tt13 - tt14);
    t7  = tt13 + tt14;
    t31 = c2 * (tt15 - tt16);
    t15 = tt15 + tt16;
    t18 = c2 * (tt17 - tt18);
    t2  = tt17 + tt18;
    t26 = c2 * (tt19 - tt20);
    t10 = tt19 + tt20;
    t22 = c2 * (tt21 - tt22);
    t6  = tt21 + tt22;
    t30 = c2 * (tt23 - tt24);
    t14 = tt23 + tt24;
    t20 = c2 * (tt25 - tt26);
    t4  = tt25 + tt26;
    t28 = c2 * (tt27 - tt28);
    t12 = tt27 + tt28;
    t24 = c2 * (tt29 - tt30);
    t8  = tt29 + tt30;
    t32 = c2 * (tt31 - tt32);
    t16 = tt31 + tt32;
    /* Do the sums */
    /* Keep on splitting the result */
    y[384] = y[128] = t9 - (x[128] = -(x[384] = t25)); /* t25, t9  */
    t10 += t26;
    t11 += t27;
    t12 += t28;
    t13 += t29;
    t14 += t30;
    t15 += t31;
    t16 += t32;
    y[320] = y[192] = t5 + t13; /* t5  */
    y[448] = y[64] = t13 + t21; /* t13 */
    x[64] = -(x[448] = t21 - (x[192] = -(x[320] = t29))); /* t29, t21 */
    t6  += t14;
    t14 += t22;
    t22 += t30;
    t7  += t15;
    t15 += t23;
    t23 += t31;
    t8  += t16;
    t16 += t24;
    t24 += t32;
    y[288] = y[224] = t3 + t7; /* t3  */
    y[352] = y[160] = t7 + t11; /* t7  */
    y[416] = y[96] = t11 + t15; /* t11 */
    y[480] = y[32] = t15 + t19; /* t15 */
    x[32] = -(x[480] = t19 + t23); /* t19 */
    x[96] = -(x[416] = t23 + t27); /* t23 */
    x[160] = -(x[352] = t27 - (x[224] = -(x[288] = t31))); /* t31, t27 */
    t4  += t8 ;
    t8  += t12;
    t12 += t16;
    t16 += t20;
    t20 += t24;
    t24 += t28;
    t28 += t32;
    y[272] = y[240] = t2 + t4; /* t2  */
    y[304] = y[208] = t4 + t6; /* t4  */
    y[336] = y[176] = t6 + t8; /* t6  */
    y[368] = y[144] = t8 + t10; /* t8  */
    y[400] = y[112] = t10 + t12; /* t10 */
    y[432] = y[80] = t12 + t14; /* t12 */
    y[464] = y[48] = t14 + t16; /* t14 */
    y[496] = y[16] = t16 + t18; /* t16 */
    x[16] = -(x[496] = t18 + t20); /* t18 */
    x[48] = -(x[464] = t20 + t22); /* t20 */
    x[80] = -(x[432] = t22 + t24); /* t22 */
    x[112] = -(x[400] = t24 + t26); /* t24 */
    x[144] = -(x[368] = t26 + t28); /* t26 */
    x[176] = -(x[336] = t28 + t30); /* t28 */
    x[208] = -(x[304] = t30 - (x[240] = -(x[272] = t32))); /* t32, t30 */
    /* Note that to be really complete, the DCT should multiply t1 by sqrt(2)/N
       and t2 to t32 by 2/N, and would take 321 additions and multiplications.
       But that's unuseful in this application. */
}


/*****************************************************************************
 * PCM: Pulse Code Modulation
 *****************************************************************************
 * Compute 32 PCM samples with a convolution product
 *****************************************************************************/

void PCM(adec_bank_t *b, s16 **pcm, int jump)
{
    /* scale factor */
#define F 32768
    /* These values are not in the same order as in Annex 3-B.3 of the ISO/IEC
       DIS 11172-3 */
    static const float c[512] = {
        0.000000000 * F, -0.000442505 * F,  0.003250122 * F, -0.007003784 * F,
        0.031082153 * F, -0.078628540 * F,  0.100311279 * F, -0.572036743 * F,
        1.144989014 * F,  0.572036743 * F,  0.100311279 * F,  0.078628540 * F,
        0.031082153 * F,  0.007003784 * F,  0.003250122 * F,  0.000442505 * F,
       -0.000015259 * F, -0.000473022 * F,  0.003326416 * F, -0.007919312 * F,
        0.030517578 * F, -0.084182739 * F,  0.090927124 * F, -0.600219727 * F,
        1.144287109 * F,  0.543823242 * F,  0.108856201 * F,  0.073059082 * F,
        0.031478882 * F,  0.006118774 * F,  0.003173828 * F,  0.000396729 * F,
       -0.000015259 * F, -0.000534058 * F,  0.003387451 * F, -0.008865356 * F,
        0.029785156 * F, -0.089706421 * F,  0.080688477 * F, -0.628295898 * F,
        1.142211914 * F,  0.515609741 * F,  0.116577148 * F,  0.067520142 * F,
        0.031738281 * F,  0.005294800 * F,  0.003082275 * F,  0.000366211 * F,
       -0.000015259 * F, -0.000579834 * F,  0.003433228 * F, -0.009841919 * F,
        0.028884888 * F, -0.095169067 * F,  0.069595337 * F, -0.656219482 * F,
        1.138763428 * F,  0.487472534 * F,  0.123474121 * F,  0.061996460 * F,
        0.031845093 * F,  0.004486084 * F,  0.002990723 * F,  0.000320435 * F,
       -0.000015259 * F, -0.000625610 * F,  0.003463745 * F, -0.010848999 * F,
        0.027801514 * F, -0.100540161 * F,  0.057617188 * F, -0.683914185 * F,
        1.133926392 * F,  0.459472656 * F,  0.129577637 * F,  0.056533813 * F,
        0.031814575 * F,  0.003723145 * F,  0.002899170 * F,  0.000289917 * F,
       -0.000015259 * F, -0.000686646 * F,  0.003479004 * F, -0.011886597 * F,
        0.026535034 * F, -0.105819702 * F,  0.044784546 * F, -0.711318970 * F,
        1.127746582 * F,  0.431655884 * F,  0.134887695 * F,  0.051132202 * F,
        0.031661987 * F,  0.003005981 * F,  0.002792358 * F,  0.000259399 * F,
       -0.000015259 * F, -0.000747681 * F,  0.003479004 * F, -0.012939453 * F,
        0.025085449 * F, -0.110946655 * F,  0.031082153 * F, -0.738372803 * F,
        1.120223999 * F,  0.404083252 * F,  0.139450073 * F,  0.045837402 * F,
        0.031387329 * F,  0.002334595 * F,  0.002685547 * F,  0.000244141 * F,
       -0.000030518 * F, -0.000808716 * F,  0.003463745 * F, -0.014022827 * F,
        0.023422241 * F, -0.115921021 * F,  0.016510010 * F, -0.765029907 * F,
        1.111373901 * F,  0.376800537 * F,  0.143264771 * F,  0.040634155 * F,
        0.031005859 * F,  0.001693726 * F,  0.002578735 * F,  0.000213623 * F,
       -0.000030518 * F, -0.000885010 * F,  0.003417969 * F, -0.015121460 * F,
        0.021575928 * F, -0.120697021 * F,  0.001068115 * F, -0.791213989 * F,
        1.101211548 * F,  0.349868774 * F,  0.146362305 * F,  0.035552979 * F,
        0.030532837 * F,  0.001098633 * F,  0.002456665 * F,  0.000198364 * F,
       -0.000030518 * F, -0.000961304 * F,  0.003372192 * F, -0.016235352 * F,
        0.019531250 * F, -0.125259399 * F, -0.015228271 * F, -0.816864014 * F,
        1.089782715 * F,  0.323318481 * F,  0.148773193 * F,  0.030609131 * F,
        0.029937744 * F,  0.000549316 * F,  0.002349854 * F,  0.000167847 * F,
       -0.000030518 * F, -0.001037598 * F,  0.003280640 * F, -0.017349243 * F,
        0.017257690 * F, -0.129562378 * F, -0.032379150 * F, -0.841949463 * F,
        1.077117920 * F,  0.297210693 * F,  0.150497437 * F,  0.025817871 * F,
        0.029281616 * F,  0.000030518 * F,  0.002243042 * F,  0.000152588 * F,
       -0.000045776 * F, -0.001113892 * F,  0.003173828 * F, -0.018463135 * F,
        0.014801025 * F, -0.133590698 * F, -0.050354004 * F, -0.866363525 * F,
        1.063217163 * F,  0.271591187 * F,  0.151596069 * F,  0.021179199 * F,
        0.028533936 * F, -0.000442505 * F,  0.002120972 * F,  0.000137329 * F,
       -0.000045776 * F, -0.001205444 * F,  0.003051758 * F, -0.019577026 * F,
        0.012115479 * F, -0.137298584 * F, -0.069168091 * F, -0.890090942 * F,
        1.048156738 * F,  0.246505737 * F,  0.152069092 * F,  0.016708374 * F,
        0.027725220 * F, -0.000869751 * F,  0.002014160 * F,  0.000122070 * F,
       -0.000061035 * F, -0.001296997 * F,  0.002883911 * F, -0.020690918 * F,
        0.009231567 * F, -0.140670776 * F, -0.088775635 * F, -0.913055420 * F,
        1.031936646 * F,  0.221984863 * F,  0.151962280 * F,  0.012420654 * F,
        0.026840210 * F, -0.001266479 * F,  0.001907349 * F,  0.000106812 * F,
       -0.000061035 * F, -0.001388550 * F,  0.002700806 * F, -0.021789551 * F,
        0.006134033 * F, -0.143676758 * F, -0.109161377 * F, -0.935195923 * F,
        1.014617920 * F,  0.198059082 * F,  0.151306152 * F,  0.008316040 * F,
        0.025909424 * F, -0.001617432 * F,  0.001785278 * F,  0.000106812 * F,
       -0.000076294 * F, -0.001480103 * F,  0.002487183 * F, -0.022857666 * F,
        0.002822876 * F, -0.146255493 * F, -0.130310059 * F, -0.956481934 * F,
        0.996246338 * F,  0.174789429 * F,  0.150115967 * F,  0.004394531 * F,
        0.024932861 * F, -0.001937866 * F,  0.001693726 * F,  0.000091553 * F,
       -0.000076294 * F, -0.001586914 * F,  0.002227783 * F, -0.023910522 * F,
       -0.000686646 * F, -0.148422241 * F, -0.152206421 * F, -0.976852417 * F,
        0.976852417 * F,  0.152206421 * F,  0.148422241 * F,  0.000686646 * F,
        0.023910522 * F, -0.002227783 * F,  0.001586914 * F,  0.000076294 * F,
       -0.000091553 * F, -0.001693726 * F,  0.001937866 * F, -0.024932861 * F,
       -0.004394531 * F, -0.150115967 * F, -0.174789429 * F, -0.996246338 * F,
        0.956481934 * F,  0.130310059 * F,  0.146255493 * F, -0.002822876 * F,
        0.022857666 * F, -0.002487183 * F,  0.001480103 * F,  0.000076294 * F,
       -0.000106812 * F, -0.001785278 * F,  0.001617432 * F, -0.025909424 * F,
       -0.008316040 * F, -0.151306152 * F, -0.198059082 * F, -1.014617920 * F,
        0.935195923 * F,  0.109161377 * F,  0.143676758 * F, -0.006134033 * F,
        0.021789551 * F, -0.002700806 * F,  0.001388550 * F,  0.000061035 * F,
       -0.000106812 * F, -0.001907349 * F,  0.001266479 * F, -0.026840210 * F,
       -0.012420654 * F, -0.151962280 * F, -0.221984863 * F, -1.031936646 * F,
        0.913055420 * F,  0.088775635 * F,  0.140670776 * F, -0.009231567 * F,
        0.020690918 * F, -0.002883911 * F,  0.001296997 * F,  0.000061035 * F,
       -0.000122070 * F, -0.002014160 * F,  0.000869751 * F, -0.027725220 * F,
       -0.016708374 * F, -0.152069092 * F, -0.246505737 * F, -1.048156738 * F,
        0.890090942 * F,  0.069168091 * F,  0.137298584 * F, -0.012115479 * F,
        0.019577026 * F, -0.003051758 * F,  0.001205444 * F,  0.000045776 * F,
       -0.000137329 * F, -0.002120972 * F,  0.000442505 * F, -0.028533936 * F,
       -0.021179199 * F, -0.151596069 * F, -0.271591187 * F, -1.063217163 * F,
        0.866363525 * F,  0.050354004 * F,  0.133590698 * F, -0.014801025 * F,
        0.018463135 * F, -0.003173828 * F,  0.001113892 * F,  0.000045776 * F,
       -0.000152588 * F, -0.002243042 * F, -0.000030518 * F, -0.029281616 * F,
       -0.025817871 * F, -0.150497437 * F, -0.297210693 * F, -1.077117920 * F,
        0.841949463 * F,  0.032379150 * F,  0.129562378 * F, -0.017257690 * F,
        0.017349243 * F, -0.003280640 * F,  0.001037598 * F,  0.000030518 * F,
       -0.000167847 * F, -0.002349854 * F, -0.000549316 * F, -0.029937744 * F,
       -0.030609131 * F, -0.148773193 * F, -0.323318481 * F, -1.089782715 * F,
        0.816864014 * F,  0.015228271 * F,  0.125259399 * F, -0.019531250 * F,
        0.016235352 * F, -0.003372192 * F,  0.000961304 * F,  0.000030518 * F,
       -0.000198364 * F, -0.002456665 * F, -0.001098633 * F, -0.030532837 * F,
       -0.035552979 * F, -0.146362305 * F, -0.349868774 * F, -1.101211548 * F,
        0.791213989 * F, -0.001068115 * F,  0.120697021 * F, -0.021575928 * F,
        0.015121460 * F, -0.003417969 * F,  0.000885010 * F,  0.000030518 * F,
       -0.000213623 * F, -0.002578735 * F, -0.001693726 * F, -0.031005859 * F,
       -0.040634155 * F, -0.143264771 * F, -0.376800537 * F, -1.111373901 * F,
        0.765029907 * F, -0.016510010 * F,  0.115921021 * F, -0.023422241 * F,
        0.014022827 * F, -0.003463745 * F,  0.000808716 * F,  0.000030518 * F,
       -0.000244141 * F, -0.002685547 * F, -0.002334595 * F, -0.031387329 * F,
       -0.045837402 * F, -0.139450073 * F, -0.404083252 * F, -1.120223999 * F,
        0.738372803 * F, -0.031082153 * F,  0.110946655 * F, -0.025085449 * F,
        0.012939453 * F, -0.003479004 * F,  0.000747681 * F,  0.000015259 * F,
       -0.000259399 * F, -0.002792358 * F, -0.003005981 * F, -0.031661987 * F,
       -0.051132202 * F, -0.134887695 * F, -0.431655884 * F, -1.127746582 * F,
        0.711318970 * F, -0.044784546 * F,  0.105819702 * F, -0.026535034 * F,
        0.011886597 * F, -0.003479004 * F,  0.000686646 * F,  0.000015259 * F,
       -0.000289917 * F, -0.002899170 * F, -0.003723145 * F, -0.031814575 * F,
       -0.056533813 * F, -0.129577637 * F, -0.459472656 * F, -1.133926392 * F,
        0.683914185 * F, -0.057617188 * F,  0.100540161 * F, -0.027801514 * F,
        0.010848999 * F, -0.003463745 * F,  0.000625610 * F,  0.000015259 * F,
       -0.000320435 * F, -0.002990723 * F, -0.004486084 * F, -0.031845093 * F,
       -0.061996460 * F, -0.123474121 * F, -0.487472534 * F, -1.138763428 * F,
        0.656219482 * F, -0.069595337 * F,  0.095169067 * F, -0.028884888 * F,
        0.009841919 * F, -0.003433228 * F,  0.000579834 * F,  0.000015259 * F,
       -0.000366211 * F, -0.003082275 * F, -0.005294800 * F, -0.031738281 * F,
       -0.067520142 * F, -0.116577148 * F, -0.515609741 * F, -1.142211914 * F,
        0.628295898 * F, -0.080688477 * F,  0.089706421 * F, -0.029785156 * F,
        0.008865356 * F, -0.003387451 * F,  0.000534058 * F,  0.000015259 * F,
       -0.000396729 * F, -0.003173828 * F, -0.006118774 * F, -0.031478882 * F,
       -0.073059082 * F, -0.108856201 * F, -0.543823242 * F, -1.144287109 * F,
        0.600219727 * F, -0.090927124 * F,  0.084182739 * F, -0.030517578 * F,
        0.007919312 * F, -0.003326416 * F,  0.000473022 * F,  0.000015259 * F
        };
#undef F
    int i;
    float tmp, *v;
    const float *f;

    f = c;

    switch(b->pos) {
        case 0:
            v = b->actual;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    /* ceiling saturation */
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        /* floor saturation */
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 1:
            v = b->actual + 1;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 2:
            v = b->actual + 2;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 3:
            v = b->actual + 3;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 4:
            v = b->actual + 4;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 5:
            v = b->actual + 5;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 6:
            v = b->actual + 6;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 7:
            v = b->actual + 7;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 8:
            v = b->actual + 8;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 9:
            v = b->actual + 9;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 10:
            v = b->actual + 10;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 11:
            v = b->actual + 11;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 12:
            v = b->actual + 12;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 13:
            v = b->actual + 13;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 14:
            v = b->actual + 14;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v;
                v += 15;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 15;
            }
            break;
        case 15:
            v = b->actual + 15;
            for(i=0; i<32; i++) {
                tmp = *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                tmp += *f++ * *v--;
                if((tmp += *f++ * *v) > 32767)
                    **pcm = 0x7FFF;
                else
                    if(tmp < -32768)
                        **pcm = 0x8000;
                    else
                        **pcm = (s16)tmp;
                *pcm += jump;
                v += 31;
            }
            break;
    }

    /* Set the next position in the filter bank */
    b->pos++;
    b->pos &= 15;
    b->actual = (b->actual == b->v1 ? b->v2 : b->v1);
}
