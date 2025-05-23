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

#ifndef ANDROID_AUDIO_PRIMITIVES_H
#define ANDROID_AUDIO_PRIMITIVES_H

#define HAVE_LITTLE_ENDIAN

#define INLINE __inline //TODO: if msvc/gcc?

#include <stdint.h>
#include <stdlib.h>

/* The memcpy_* conversion routines are designed to work in-place on same dst as src
 * buffers only if the types shrink on copy, with the exception of memcpy_to_i16_from_u8().
 * This allows the loops to go upwards for faster cache access (and may be more flexible
 * for future optimization later).
 */

/**
 * Dither and clamp pairs of 32-bit input samples (sums) to 16-bit output samples (out).
 * Each 32-bit input sample can be viewed as a signed fixed-point Q19.12 of which the
 * .12 fraction bits are dithered and the 19 integer bits are clamped to signed 16 bits.
 * Alternatively the input can be viewed as Q4.27, of which the lowest .12 of the fraction
 * is dithered and the remaining fraction is converted to the output Q.15, with clamping
 * on the 4 integer guard bits.
 *
 * For interleaved stereo, c is the number of sample pairs,
 * and out is an array of interleaved pairs of 16-bit samples per channel.
 * For mono, c is the number of samples / 2, and out is an array of 16-bit samples.
 * The name "dither" is a misnomer; the current implementation does not actually dither
 * but uses truncation.  This may change.
 * The out and sums buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void ditherAndClamp(int32_t* out, const int32_t *sums, size_t c);

/* Expand and copy samples from unsigned 8-bit offset by 0x80 to signed 16-bit.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_i16_from_u8(int16_t *dst, const uint8_t *src, size_t count);

/* Shrink and copy samples from signed 16-bit to unsigned 8-bit offset by 0x80.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 * The conversion is done by truncation, without dithering, so it loses resolution.
 */
void memcpy_to_u8_from_i16(uint8_t *dst, const int16_t *src, size_t count);

/* Shrink and copy samples from signed 32-bit fixed-point Q0.31 to signed 16-bit Q0.15.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 * The conversion is done by truncation, without dithering, so it loses resolution.
 */
void memcpy_to_i16_from_i32(int16_t *dst, const int32_t *src, size_t count);

/* Shrink and copy samples from single-precision floating-point to signed 16-bit.
 * Each float should be in the range -1.0 to 1.0.  Values outside that range are clamped,
 * refer to clamp16_from_float().
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 * The conversion is done by truncation, without dithering, so it loses resolution.
 */
void memcpy_to_i16_from_float(int16_t *dst, const float *src, size_t count);
void memcpy_to_i16_from_float_with_ramp(int16_t *dst, const float *src, size_t count, const float start_gain, const float end_gain);

/* Copy samples from signed fixed-point 32-bit Q4.27 to single-precision floating-point.
 * The nominal output float range is [-1.0, 1.0] if the fixed-point range is
 * [0xf8000000, 0x07ffffff].  The full float range is [-16.0, 16.0].  Note the closed range
 * at 1.0 and 16.0 is due to rounding on conversion to float. See float_from_q4_27() for details.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_float_from_q4_27(float *dst, const int32_t *src, size_t count);

/* Copy samples from signed fixed-point 16 bit Q0.15 to single-precision floating-point.
 * The output float range is [-1.0, 1.0) for the fixed-point range [0x8000, 0x7fff].
 * No rounding is needed as the representation is exact.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must be completely separate.
 */
void memcpy_to_float_from_i16(float *dst, const int16_t *src, size_t count);

/* Copy samples from signed fixed-point packed 24 bit Q0.23 to single-precision floating-point.
 * The packed 24 bit input is stored in native endian format in a uint8_t byte array.
 * The output float range is [-1.0, 1.0) for the fixed-point range [0x800000, 0x7fffff].
 * No rounding is needed as the representation is exact.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must be completely separate.
 */
void memcpy_to_float_from_p24(float *dst, const uint8_t *src, size_t count);

/* Copy samples from signed fixed-point packed 24 bit Q0.23 to signed fixed point 16 bit Q0.15.
 * The packed 24 bit output is stored in native endian format in a uint8_t byte array.
 * The data is truncated without rounding.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_i16_from_p24(int16_t *dst, const uint8_t *src, size_t count);

/* Copy samples from signed fixed point 16 bit Q0.15 to signed fixed-point packed 24 bit Q0.23.
 * The packed 24 bit output is assumed to be a native-endian uint8_t byte array.
 * The output data range is [0x800000, 0x7fff00] (not full).
 * Nevertheless there is no DC offset on the output, if the input has no DC offset.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must be completely separate.
 */
void memcpy_to_p24_from_i16(uint8_t *dst, const int16_t *src, size_t count);

/* Copy samples from single-precision floating-point to signed fixed-point packed 24 bit Q0.23.
 * The packed 24 bit output is assumed to be a native-endian uint8_t byte array.
 * The data is clamped and rounded to nearest, ties away from zero. See clamp24_from_float()
 * for details.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_p24_from_float(uint8_t *dst, const float *src, size_t count);
void memcpy_to_p24_from_float_with_ramp(uint8_t *dst, const float *src, size_t count, const float start_gain, const float end_gain);

/* Copy samples from signed fixed-point 32-bit Q8.23 to signed fixed-point packed 24 bit Q0.23.
 * The packed 24 bit output is assumed to be a native-endian uint8_t byte array.
 * The data is clamped to the range is [0x800000, 0x7fffff].
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must be completely separate.
 */
void memcpy_to_p24_from_q8_23(uint8_t *dst, const int32_t *src, size_t count);

/* Copy samples from signed fixed point 16-bit Q0.15 to signed fixed-point 32-bit Q8.23.
 * The output data range is [0xff800000, 0x007fff00] at intervals of 0x100.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must be completely separate.
 */
void memcpy_to_q8_23_from_i16(int32_t *dst, const int16_t *src, size_t count);

/* Copy samples from single-precision floating-point to signed fixed-point 32-bit Q8.23.
 * This copy will clamp the Q8.23 representation to [0xff800000, 0x007fffff] even though there
 * are guard bits available. Fractional lsb is rounded to nearest, ties away from zero.
 * See clamp24_from_float() for details.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_q8_23_from_float_with_clamp(int32_t *dst, const float *src, size_t count);

/* Copy samples from single-precision floating-point to signed fixed-point 32-bit Q4.27.
 * The conversion will use the full available Q4.27 range, including guard bits.
 * Fractional lsb is rounded to nearest, ties away from zero.
 * q0.27 versions will limit the signal to +-1
 * See clampq4_27_from_float() for details.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_q4_27_from_float(int32_t *dst, const float *src, size_t count);
void memcpy_to_q4_27_from_float_with_ramp(int32_t *dst, const float *src, size_t count, const float start_gain, const float end_gain);
void memcpy_to_q0_27_from_float(int32_t *dst, const float *src, size_t count);
void memcpy_to_q0_27_from_float_with_ramp(int32_t *dst, const float *src, size_t count, const float start_gain, const float end_gain);

/* Copy samples from signed fixed-point 32-bit Q8.23 to signed fixed point 16-bit Q0.15.
 * The data is clamped, and truncated without rounding.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_i16_from_q8_23(int16_t *dst, const int32_t *src, size_t count);

/* Copy samples from signed fixed-point 32-bit Q8.23 to single-precision floating-point.
 * The nominal output float range is [-1.0, 1.0) for the fixed-point
 * range [0xff800000, 0x007fffff]. The maximum output float range is [-256.0, 256.0).
 * No rounding is needed as the representation is exact for nominal values.
 * Rounding for overflow values is to nearest, ties to even.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_float_from_q8_23(float *dst, const int32_t *src, size_t count);

/* Copy samples from signed fixed point 16-bit Q0.15 to signed fixed-point 32-bit Q0.31.
 * The output data range is [0x80000000, 0x7fff0000] at intervals of 0x10000.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must be completely separate.
 */
void memcpy_to_i32_from_i16(int32_t *dst, const int16_t *src, size_t count);

/* Copy samples from single-precision floating-point to signed fixed-point 32-bit Q0.31.
 * If rounding is needed on truncation, the fractional lsb is rounded to nearest,
 * ties away from zero. See clamp32_from_float() for details.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_i32_from_float(int32_t *dst, const float *src, size_t count);

/* Copy samples from signed fixed-point 32-bit Q0.31 to single-precision floating-point.
 * The float range is [-1.0, 1.0] for the fixed-point range [0x80000000, 0x7fffffff].
 * Rounding is done according to float_from_i32().
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of samples to copy
 * The destination and source buffers must either be completely separate (non-overlapping), or
 * they must both start at the same address.  Partially overlapping buffers are not supported.
 */
void memcpy_to_float_from_i32(float *dst, const int32_t *src, size_t count);

/* Downmix pairs of interleaved stereo input 16-bit samples to mono output 16-bit samples.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of stereo frames to downmix
 * The destination and source buffers must be completely separate (non-overlapping).
 * The current implementation truncates the sum rather than dither, but this may change.
 */
void downmix_to_mono_i16_from_stereo_i16(int16_t *dst, const int16_t *src, size_t count);

/* Upmix mono input 16-bit samples to pairs of interleaved stereo output 16-bit samples by
 * duplicating.
 * Parameters:
 *  dst     Destination buffer
 *  src     Source buffer
 *  count   Number of mono samples to upmix
 * The destination and source buffers must be completely separate (non-overlapping).
 */
void upmix_to_stereo_i16_from_mono_i16(int16_t *dst, const int16_t *src, size_t count);

/* Return the total number of non-zero 32-bit samples */
size_t nonZeroMono32(const int32_t *samples, size_t count);

/* Return the total number of non-zero 16-bit samples */
size_t nonZeroMono16(const int16_t *samples, size_t count);

/* Return the total number of non-zero stereo frames, where a frame is considered non-zero
 * if either of its constituent 32-bit samples is non-zero
 */
size_t nonZeroStereo32(const int32_t *frames, size_t count);

/* Return the total number of non-zero stereo frames, where a frame is considered non-zero
 * if either of its constituent 16-bit samples is non-zero
 */
size_t nonZeroStereo16(const int16_t *frames, size_t count);

/**
 * Clamp (aka hard limit or clip) a signed 32-bit sample to 16-bit range.
 */
static INLINE int16_t clamp16(int32_t sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

/*
 * Convert a IEEE 754 single precision float [-1.0, 1.0) to int16_t [-32768, 32767]
 * with clamping.  Note the open bound at 1.0, values within 1/65536 of 1.0 map
 * to 32767 instead of 32768 (early clamping due to the smaller positive integer subrange).
 *
 * Values outside the range [-1.0, 1.0) are properly clamped to -32768 and 32767,
 * including -Inf and +Inf. NaN will generally be treated either as -32768 or 32767,
 * depending on the sign bit inside NaN (whose representation is not unique).
 * Nevertheless, strictly speaking, NaN behavior should be considered undefined.
 *
 * Rounding of 0.5 lsb is to even (default for IEEE 754).
 */
static INLINE int16_t clamp16_from_float(float f)
{
    /* Offset is used to expand the valid range of [-1.0, 1.0) into the 16 lsbs of the
     * floating point significand. The normal shift is 3<<22, but the -15 offset
     * is used to multiply by 32768.
     */
    static const float offset = (float)(3 << (22 - 15));
    /* zero = (0x10f << 22) =  0x43c00000 (not directly used) */
    static const int32_t limneg = (0x10f << 22) /*zero*/ - 32768; /* 0x43bf8000 */
    static const int32_t limpos = (0x10f << 22) /*zero*/ + 32767; /* 0x43c07fff */

    union {
        float f;
        int32_t i;
    } u;

    u.f = f + offset; /* recenter valid range */
    /* Now the valid range is represented as integers between [limneg, limpos].
     * Clamp using the fact that float representation (as an integer) is an ordered set.
     */
    if (u.i < limneg)
        u.i = -32768;
    else if (u.i > limpos)
        u.i = 32767;
    return u.i; /* Return lower 16 bits, the part of interest in the significand. */
}


/* Convert a single-precision floating point value to a Q0.23 integer value, stored in a
 * 32 bit signed integer (technically stored as Q8.23, but clamped to Q0.23).
 *
 * Rounds to nearest, ties away from 0.
 *
 * Values outside the range [-1.0, 1.0) are properly clamped to -8388608 and 8388607,
 * including -Inf and +Inf. NaN values are considered undefined, and behavior may change
 * depending on hardware and future implementation of this function.
 */
static INLINE int32_t clamp24_from_float(float f)
{
    #define scale ((float)(1 << 23)) //hack to fix "element is not constant"
    static const float limpos = 0x7fffff / scale;
    static const float limneg = -0x800000 / scale;

    
    if (f <= limneg) {
        return -0x800000;
    } else if (f >= limpos) {
        return 0x7fffff;
    }
    f *= scale;
    
    #undef scale
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return (int32_t)(f > 0 ? f + 0.5f : f - 0.5f);
}


/* Convert a signed fixed-point 32-bit Q8.23 value to a Q0.23 integer value,
 * stored in a 32-bit signed integer (technically stored as Q8.23, but clamped to Q0.23).
 *
 * Values outside the range [-0x800000, 0x7fffff] are clamped to that range.
 */
static INLINE int32_t clamp24_from_q8_23(int32_t ival)
{
    static const int32_t limpos = 0x7fffff;
    static const int32_t limneg = -0x800000;
    if (ival < limneg) {
        return limneg;
    } else if (ival > limpos) {
        return limpos;
    } else {
        return ival;
    }
}

/* Convert a single-precision floating point value to a Q4.27 integer value.
 * Rounds to nearest, ties away from 0.
 *
 * Values outside the range [-16.0, 16.0) are properly clamped to -2147483648 and 2147483647,
 * including -Inf and +Inf. NaN values are considered undefined, and behavior may change
 * depending on hardware and future implementation of this function.
 */
static INLINE int32_t clampq4_27_from_float(float f)
{
    static const float scale = (float)(1UL << 27);
    static const float limpos = 16.;
    static const float limneg = -16.;

    if (f <= limneg) {
        return 0x80000000;
    } else if (f >= limpos) {
        return 0x7fffffff;
    }
    f *= scale;
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return (int32_t)(f > 0 ? f + 0.5f : f - 0.5f);
}

/* Convert a single-precision floating point value to a Q0.27 integer value.
* Rounds to nearest, ties away from 0.
*
* Values outside the range [-1.0, 1.0) are properly clamped to -134217728 and 134217727,
* including -Inf and +Inf. NaN values are considered undefined, and behavior may change
* depending on hardware and future implementation of this function.
*/
static INLINE int32_t clampq0_27_from_float(float f)
{
    static const float scale = (float)(1UL << 27);
    static const float limpos = 1.;
    static const float limneg = -1.;

    if (f <= limneg) {
        return 0xf8000000;
    }
    else if (f >= limpos) {
        return 0x07ffffff;
    }
    f *= scale;
    /* integer conversion is through truncation (though int to float is not).
    * ensure that we round to nearest, ties away from 0.
    */
    return (int32_t)(f > 0 ? f + 0.5f : f - 0.5f);
}


/* Convert a single-precision floating point value to a Q0.31 integer value.
 * Rounds to nearest, ties away from 0.
 *
 * Values outside the range [-1.0, 1.0) are properly clamped to -2147483648 and 2147483647,
 * including -Inf and +Inf. NaN values are considered undefined, and behavior may change
 * depending on hardware and future implementation of this function.
 */
static INLINE int32_t clamp32_from_float(float f)
{
    static const float scale = (float)(1UL << 31);
    static const float limpos = 1.;
    static const float limneg = -1.;

    if (f <= limneg) {
        return 0x80000000;
    } else if (f >= limpos) {
        return 0x7fffffff;
    }
    f *= scale;
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return (int32_t)(f > 0 ? f + 0.5f : f - 0.5f);
}

/* Convert a signed fixed-point 32-bit Q4.27 value to single-precision floating-point.
 * The nominal output float range is [-1.0, 1.0] if the fixed-point range is
 * [0xf8000000, 0x07ffffff].  The full float range is [-16.0, 16.0].
 *
 * Note the closed range at 1.0 and 16.0 is due to rounding on conversion to float.
 * In more detail: if the fixed-point integer exceeds 24 bit significand of single
 * precision floating point, the 0.5 lsb in the significand conversion will round
 * towards even, as per IEEE 754 default.
 */
static INLINE float float_from_q4_27(int32_t ival)
{
    /* The scale factor is the reciprocal of the fractional bits.
     *
     * Since the scale factor is a power of 2, the scaling is exact, and there
     * is no rounding due to the multiplication - the bit pattern is preserved.
     * However, there may be rounding due to the fixed-point to float conversion,
     * as described above.
     */
    static const float scale = 1. / (float)(1UL << 27);

    return ival * scale;
}

/* Convert an unsigned fixed-point 32-bit U4.28 value to single-precision floating-point.
 * The nominal output float range is [0.0, 1.0] if the fixed-point range is
 * [0x00000000, 0x10000000].  The full float range is [0.0, 16.0].
 *
 * Note the closed range at 1.0 and 16.0 is due to rounding on conversion to float.
 * In more detail: if the fixed-point integer exceeds 24 bit significand of single
 * precision floating point, the 0.5 lsb in the significand conversion will round
 * towards even, as per IEEE 754 default.
 */
static INLINE float float_from_u4_28(uint32_t uval)
{
    static const float scale = 1. / (float)(1UL << 28);

    return uval * scale;
}

/* Convert an unsigned fixed-point 16-bit U4.12 value to single-precision floating-point.
 * The nominal output float range is [0.0, 1.0] if the fixed-point range is
 * [0x0000, 0x1000].  The full float range is [0.0, 16.0).
 */
static INLINE float float_from_u4_12(uint16_t uval)
{
    static const float scale = 1. / (float)(1UL << 12);

    return uval * scale;
}

#if 0
/* Convert a single-precision floating point value to a U4.28 integer value.
 * Rounds to nearest, ties away from 0.
 *
 * Values outside the range [0, 16.0] are properly clamped to [0, 4294967295]
 * including -Inf and +Inf. NaN values are considered undefined, and behavior may change
 * depending on hardware and future implementation of this function.
 */
static INLINE uint32_t u4_28_from_float(float f)
{
    static const float scale = (float)(1 << 28);
    static const float limpos = 0xffffffffUL / scale;

    if (f <= 0.) {
        return 0;
    } else if (f >= limpos) {
        return 0xffffffff;
    }
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return f * scale + 0.5;
}

/* Convert a single-precision floating point value to a U4.12 integer value.
 * Rounds to nearest, ties away from 0.
 *
 * Values outside the range [0, 16.0) are properly clamped to [0, 65535]
 * including -Inf and +Inf. NaN values are considered undefined, and behavior may change
 * depending on hardware and future implementation of this function.
 */
static INLINE uint16_t u4_12_from_float(float f)
{
    static const float scale = (float)(1 << 12);
    static const float limpos = 0xffff / scale;

    if (f <= 0.) {
        return 0;
    } else if (f >= limpos) {
        return 0xffff;
    }
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return f * scale + 0.5;
}
#endif

/* Convert a signed fixed-point 16-bit Q0.15 value to single-precision floating-point.
 * The output float range is [-1.0, 1.0) for the fixed-point range
 * [0x8000, 0x7fff].
 *
 * There is no rounding, the conversion and representation is exact.
 */
static INLINE float float_from_i16(int16_t ival)
{
    /* The scale factor is the reciprocal of the nominal 16 bit integer
     * half-sided range (32768).
     *
     * Since the scale factor is a power of 2, the scaling is exact, and there
     * is no rounding due to the multiplication - the bit pattern is preserved.
     */
    static const float scale = 1. / (float)(1UL << 15);

    return ival * scale;
}

/* Convert a packed 24bit Q0.23 value stored native-endian in a uint8_t ptr
 * to a signed fixed-point 32 bit integer Q0.31 value. The output Q0.31 range
 * is [0x80000000, 0x7fffff00] for the fixed-point range [0x800000, 0x7fffff].
 * Even though the output range is limited on the positive side, there is no
 * DC offset on the output, if the input has no DC offset.
 *
 * Avoid relying on the limited output range, as future implementations may go
 * to full range.
 */
static INLINE int32_t i32_from_p24(const uint8_t *packed24)
{
    /* convert to 32b */
#if defined(HAVE_BIG_ENDIAN) == defined(HAVE_LITTLE_ENDIAN)
    /* check to see if we have exactly one or the other android endian flags set. */
#error "Either HAVE_LITTLE_ENDIAN or HAVE_BIG_ENDIAN must be defined"
#elif defined(HAVE_BIG_ENDIAN)
    return (packed24[2] << 8) | (packed24[1] << 16) | (packed24[0] << 24);
#else /* HAVE_LITTLE_ENDIAN */
    return (packed24[0] << 8) | (packed24[1] << 16) | (packed24[2] << 24);
#endif
}

/* Convert a 32-bit Q0.31 value to single-precision floating-point.
 * The output float range is [-1.0, 1.0] for the fixed-point range
 * [0x80000000, 0x7fffffff].
 *
 * Rounding may occur in the least significant 8 bits for large fixed point
 * values due to storage into the 24-bit floating-point significand.
 * Rounding will be to nearest, ties to even.
 */
static INLINE float float_from_i32(int32_t ival)
{
    static const float scale = 1. / (float)(1UL << 31);

    return ival * scale;
}

/* Convert a packed 24bit Q0.23 value stored native endian in a uint8_t ptr
 * to single-precision floating-point. The output float range is [-1.0, 1.0)
 * for the fixed-point range [0x800000, 0x7fffff].
 *
 * There is no rounding, the conversion and representation is exact.
 */
static INLINE float float_from_p24(const uint8_t *packed24)
{
    return float_from_i32(i32_from_p24(packed24));
}

/* Convert a 24-bit Q8.23 value to single-precision floating-point.
 * The nominal output float range is [-1.0, 1.0) for the fixed-point
 * range [0xff800000, 0x007fffff].  The maximum float range is [-256.0, 256.0).
 *
 * There is no rounding in the nominal range, the conversion and representation
 * is exact. For values outside the nominal range, rounding is to nearest, ties to even.
 */
static INLINE float float_from_q8_23(int32_t ival)
{
    static const float scale = 1. / (float)(1UL << 23);

    return ival * scale;
}

/**
 * Multiply-accumulate 16-bit terms with 32-bit result: return a + in*v.
 */
static INLINE
int32_t mulAdd(int16_t in, int16_t v, int32_t a)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    asm( "smlabb %[out], %[in], %[v], %[a] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v), [a]"r"(a)
         : );
    return out;
#else
    return a + in * (int32_t)v;
#endif
}

/**
 * Multiply 16-bit terms with 32-bit result: return in*v.
 */
static INLINE
int32_t mul(int16_t in, int16_t v)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    asm( "smulbb %[out], %[in], %[v] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v)
         : );
    return out;
#else
    return in * (int32_t)v;
#endif
}

/**
 * Similar to mulAdd, but the 16-bit terms are extracted from a 32-bit interleaved stereo pair.
 */
static INLINE
int32_t mulAddRL(int left, uint32_t inRL, uint32_t vRL, int32_t a)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    if (left) {
        asm( "smlabb %[out], %[inRL], %[vRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [vRL]"r"(vRL), [a]"r"(a)
             : );
    } else {
        asm( "smlatt %[out], %[inRL], %[vRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [vRL]"r"(vRL), [a]"r"(a)
             : );
    }
    return out;
#else
    if (left) {
        return a + (int16_t)(inRL&0xFFFF) * (int16_t)(vRL&0xFFFF);
    } else {
        return a + (int16_t)(inRL>>16) * (int16_t)(vRL>>16);
    }
#endif
}

/**
 * Similar to mul, but the 16-bit terms are extracted from a 32-bit interleaved stereo pair.
 */
static INLINE
int32_t mulRL(int left, uint32_t inRL, uint32_t vRL)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    if (left) {
        asm( "smulbb %[out], %[inRL], %[vRL] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [vRL]"r"(vRL)
             : );
    } else {
        asm( "smultt %[out], %[inRL], %[vRL] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [vRL]"r"(vRL)
             : );
    }
    return out;
#else
    if (left) {
        return (int16_t)(inRL&0xFFFF) * (int16_t)(vRL&0xFFFF);
    } else {
        return (int16_t)(inRL>>16) * (int16_t)(vRL>>16);
    }
#endif
}

#endif  // ANDROID_AUDIO_PRIMITIVES_H
