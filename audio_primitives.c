/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "audio_primitives.h"

void ditherAndClamp(int32_t* out, const int32_t *sums, size_t c)
{
    size_t i;
    for (i=0 ; i<c ; i++) {
        int32_t l = *sums++;
        int32_t r = *sums++;
        int32_t nl = l >> 12;
        int32_t nr = r >> 12;
        l = clamp16(nl);
        r = clamp16(nr);
        *out++ = (r<<16) | (l & 0xFFFF);
    }
}

void memcpy_to_i16_from_u8(int16_t *dst, const uint8_t *src, size_t count)
{
    dst += count;
    src += count;
    while (count--) {
        *--dst = (int16_t)(*--src - 0x80) << 8;
    }
}

void memcpy_to_u8_from_i16(uint8_t *dst, const int16_t *src, size_t count)
{
    while (count--) {
        *dst++ = (*src++ >> 8) + 0x80;
    }
}

void memcpy_to_i16_from_i32(int16_t *dst, const int32_t *src, size_t count)
{
    while (count--) {
        *dst++ = *src++ >> 16;
    }
}

void memcpy_to_i16_from_float(int16_t *dst, const float *src, size_t count)
{
    while (count--) {
        *dst++ = clamp16_from_float(*src++);
    }
}


void memcpy_to_i16_from_float_with_ramp(int16_t *dst, const float *src, size_t count, const float start_gain, const float end_gain)
{
    const float inc = (end_gain - start_gain) / count;
    float current_gain = start_gain;
    while (count--) {
        *dst++ = clamp16_from_float(*src++ * current_gain);
        current_gain += inc;
    }
}


void memcpy_to_float_from_q4_27(float *dst, const int32_t *src, size_t count)
{
    while (count--) {
        *dst++ = float_from_q4_27(*src++);
    }
}

void memcpy_to_float_from_i16(float *dst, const int16_t *src, size_t count)
{
    while (count--) {
        *dst++ = float_from_i16(*src++);
    }
}

void memcpy_to_float_from_p24(float *dst, const uint8_t *src, size_t count)
{
    while (count--) {
        *dst++ = float_from_p24(src);
        src += 3;
    }
}

void memcpy_to_i16_from_p24(int16_t *dst, const uint8_t *src, size_t count)
{
    while (count--) {
#ifdef HAVE_BIG_ENDIAN
        *dst++ = src[1] | (src[0] << 8);
#else
        *dst++ = src[1] | (src[2] << 8);
#endif
        src += 3;
    }
}

void memcpy_to_p24_from_i16(uint8_t *dst, const int16_t *src, size_t count)
{
    while (count--) {
#ifdef HAVE_BIG_ENDIAN
        *dst++ = *src >> 8;
        *dst++ = *src++;
        *dst++ = 0;
#else
        *dst++ = 0;
        *dst++ = (uint8_t)(*src);
        *dst++ = *src++ >> 8;
#endif
    }
}

void memcpy_to_p24_from_float(uint8_t *dst, const float *src, size_t count)
{
    while (count--) {
        int32_t ival = clamp24_from_float(*src++);

#ifdef HAVE_BIG_ENDIAN
        *dst++ = ival >> 16;
        *dst++ = ival >> 8;
        *dst++ = ival;
#else
        *dst++ = ival;
        *dst++ = ival >> 8;
        *dst++ = ival >> 16;
#endif
    }
}


void memcpy_to_p24_from_float_with_ramp(uint8_t *dst, const float *src, size_t count, const float start_gain, const float end_gain)
{
    const float inc = (end_gain - start_gain) / count;
    float current_gain = start_gain;

    while (count--) {
        int32_t ival = clamp24_from_float(*src++ * current_gain);
        current_gain += inc;
#ifdef HAVE_BIG_ENDIAN
        *dst++ = ival >> 16;
        *dst++ = ival >> 8;
        *dst++ = ival;
#else
        *dst++ = ival;
        *dst++ = ival >> 8;
        *dst++ = ival >> 16;
#endif
    }
}


void memcpy_to_p24_from_q8_23(uint8_t *dst, const int32_t *src, size_t count)
{
    while (count--) {
        int32_t ival = clamp24_from_q8_23(*src++);

#ifdef HAVE_BIG_ENDIAN
        *dst++ = ival >> 16;
        *dst++ = ival >> 8;
        *dst++ = ival;
#else
        *dst++ = ival;
        *dst++ = ival >> 8;
        *dst++ = ival >> 16;
#endif
    }
}

void memcpy_to_q8_23_from_i16(int32_t *dst, const int16_t *src, size_t count)
{
    while (count--) {
        *dst++ = (int32_t)*src++ << 8;
    }
}

void memcpy_to_q8_23_from_float_with_clamp(int32_t *dst, const float *src, size_t count)
{
    while (count--) {
        *dst++ = clamp24_from_float(*src++);
    }
}

void memcpy_to_q4_27_from_float(int32_t *dst, const float *src, size_t count)
{
    while (count--) {
        *dst++ = clampq4_27_from_float(*src++);
    }
}

void memcpy_to_q4_27_from_float_with_ramp(int32_t *dst, const float *src, size_t count, const float start_gain, const float end_gain)
{
    const float inc = (end_gain - start_gain) / count;
    float current_gain = start_gain;
    while (count--) {
        *dst++ = clampq4_27_from_float(*src++ * current_gain);
        current_gain += inc;
    }
}

void memcpy_to_q0_27_from_float(int32_t *dst, const float *src, size_t count)
{
    while (count--) {
        *dst++ = clampq0_27_from_float(*src++);
    }
}

void memcpy_to_q0_27_from_float_with_ramp(int32_t *dst, const float *src,
                                          size_t count, const float start_gain,
                                          const float end_gain)
{
    const float inc = (end_gain - start_gain) / count;
    float current_gain = start_gain;
    while (count--) {
        *dst++ = clampq0_27_from_float(*src++ * current_gain);
        current_gain += inc;
    }
}

void memcpy_to_i16_from_q8_23(int16_t *dst, const int32_t *src, size_t count)
{
    while (count--) {
        *dst++ = clamp16(*src++ >> 8);
    }
}

void memcpy_to_float_from_q8_23(float *dst, const int32_t *src, size_t count)
{
    while (count--) {
        *dst++ = float_from_q8_23(*src++);
    }
}

void memcpy_to_i32_from_i16(int32_t *dst, const int16_t *src, size_t count)
{
    while (count--) {
        *dst++ = (int32_t)*src++ << 16;
    }
}

void memcpy_to_i32_from_float(int32_t *dst, const float *src, size_t count)
{
    while (count--) {
        *dst++ = clamp32_from_float(*src++);
    }
}

void memcpy_to_float_from_i32(float *dst, const int32_t *src, size_t count)
{
    while (count--) {
        *dst++ = float_from_i32(*src++);
    }
}

void downmix_to_mono_i16_from_stereo_i16(int16_t *dst, const int16_t *src, size_t count)
{
    while (count--) {
        *dst++ = (int16_t)(((int32_t)src[0] + (int32_t)src[1]) >> 1);
        src += 2;
    }
}

void upmix_to_stereo_i16_from_mono_i16(int16_t *dst, const int16_t *src, size_t count)
{
    while (count--) {
        int32_t temp = *src++;
        dst[0] = temp;
        dst[1] = temp;
        dst += 2;
    }
}

size_t nonZeroMono32(const int32_t *samples, size_t count)
{
    size_t nonZero = 0;
    while (count-- > 0) {
        if (*samples++ != 0) {
            nonZero++;
        }
    }
    return nonZero;
}

size_t nonZeroMono16(const int16_t *samples, size_t count)
{
    size_t nonZero = 0;
    while (count-- > 0) {
        if (*samples++ != 0) {
            nonZero++;
        }
    }
    return nonZero;
}

size_t nonZeroStereo32(const int32_t *frames, size_t count)
{
    size_t nonZero = 0;
    while (count-- > 0) {
        if (frames[0] != 0 || frames[1] != 0) {
            nonZero++;
        }
        frames += 2;
    }
    return nonZero;
}

size_t nonZeroStereo16(const int16_t *frames, size_t count)
{
    size_t nonZero = 0;
    while (count-- > 0) {
        if (frames[0] != 0 || frames[1] != 0) {
            nonZero++;
        }
        frames += 2;
    }
    return nonZero;
}
