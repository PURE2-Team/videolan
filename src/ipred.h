/*
 * Copyright © 2018, VideoLAN and dav1d authors
 * Copyright © 2018, Two Orioles, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DAV1D_SRC_IPRED_H__
#define __DAV1D_SRC_IPRED_H__

#include "common/bitdepth.h"

#include "src/levels.h"

/*
 * Intra prediction.
 * - a is the angle (in degrees) for directional intra predictors. For other
 *   modes, it is ignored;
 * - topleft is the same as the argument given to dav1d_prepare_intra_edges(),
 *   see ipred_prepare.h for more detailed documentation.
 */
#define decl_angular_ipred_fn(name) \
void (name)(pixel *dst, ptrdiff_t stride, const pixel *topleft, int angle)
typedef decl_angular_ipred_fn(*angular_ipred_fn);

/*
 * Create a subsampled Y plane with the DC subtracted.
 * - w/h_pad is the edge of the width/height that extends outside the visible
 *   portion of the frame in 4px units;
 * - ac has a stride of 16.
 */
#define decl_cfl_ac_fn(name) \
void (name)(int16_t *ac, const pixel *y, ptrdiff_t stride, \
            int w_pad, int h_pad)
typedef decl_cfl_ac_fn(*cfl_ac_fn);

/*
 * dst[x,y] += alpha * ac[x,y]
 * - alpha contains a q3 scalar in [-16,16] range;
 */
#define decl_cfl_pred_1_fn(name) \
void (name)(pixel *dst, ptrdiff_t stride, \
            const int16_t *ac, const int8_t alpha, \
            const int height)
typedef decl_cfl_pred_1_fn(*cfl_pred_1_fn);

/*
 * dst[plane][x,y] += alpha[plane] * ac[x,y]
 * - alphas contains two q3 scalars (one for each plane) in [-16,16] range;
 */
#define decl_cfl_pred_fn(name) \
void (name)(pixel *u_dst, pixel *v_dst, ptrdiff_t stride, \
            const int16_t *ac, const int8_t *const alphas, \
            const int height)
typedef decl_cfl_pred_fn(*cfl_pred_fn);

/*
 * dst[x,y] = pal[idx[x,y]]
 * - palette indices are [0-7]
 */
#define decl_pal_pred_fn(name) \
void (name)(pixel *dst, ptrdiff_t stride, const uint16_t *pal, \
            const uint8_t *idx, const int w, const int h)
typedef decl_pal_pred_fn(*pal_pred_fn);

typedef struct Dav1dIntraPredDSPContext {
    angular_ipred_fn intra_pred[N_RECT_TX_SIZES][N_IMPL_INTRA_PRED_MODES];

    // chroma-from-luma
    cfl_ac_fn cfl_ac[3 /* 420, 422, 444 */][N_RECT_TX_SIZES /* chroma tx size */];
    cfl_pred_1_fn cfl_pred_1[4];
    cfl_pred_fn cfl_pred[4];

    // palette
    pal_pred_fn pal_pred;
} Dav1dIntraPredDSPContext;

void dav1d_intra_pred_dsp_init_8bpc(Dav1dIntraPredDSPContext *c);
void dav1d_intra_pred_dsp_init_10bpc(Dav1dIntraPredDSPContext *c);

#endif /* __DAV1D_SRC_IPRED_H__ */
