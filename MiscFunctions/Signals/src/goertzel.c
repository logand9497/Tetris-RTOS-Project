/* goertzel_fixed -- Detect a single tone in an audio signal.
   Copyright (C) 2022 Remington Furman
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see https://www.gnu.org/licenses/.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>

/* These macros simplify working with signed fixed point numbers.

   In this notation, only the fractional bits are tracked in the macro
   names, so a Qm.n number is referred to as a Qn number, where n is
   the number of fractional bits.  This also sidesteps the issue of
   whether m includes the sign bit or not (ARM vs TI notation).

   The usual caveats of C preprocessor macros hold here.  Beware of
   multiple evaluations, side effects, etc.
*/

/* Convert to and from doubles. */
#define Qn_FROM_DOUBLE(value, n) (lrint((value) * (1 << (n))))
#define DOUBLE_FROM_Qn(value, n) ((double)(value) / (1 << (n)))

/* The number closest to +1.0 that can be represented. */
#define ONE_Qn(n) ((1<<(n)) - 1)
/* One half (0.5). */
#define HALF_Qn(n) (1<<((n) - 1))

/* Drop n bits from x (shift right) while rounding (add one half). */
#define ROUND_OFF_Qn(x, n)                      \
    ((n > 0) ? (((x) + HALF_Qn(n)) >> n) : (x))

/* Multiply two Qn numbers, rounding to the precision of the first.
   Make sure to cast one of the arguments to the size needed to avoid
   overflow in the multiplication before shifting. */
#define MUL_Qn_Qn(x, y, xn, yn)                 \
    ROUND_OFF_Qn((x) * (y), (yn))

/* Add two Qn numbers, using the precision of the first. */
#define ADD_Qn_Qn(x, y, xn, yn)                 \
    ((xn) > (yn) ? (x) + ((y) << ((xn)-(yn))) : \
     (x) + ROUND_OFF_Qn((y), ((xn)-(yn))))

typedef struct {
    int16_t real;
    int16_t imag;
} cint16_t;

/* Return a larger type here, because a complex point outside of the
   unit circle will have a larger magnitude. */
int32_t cint16_abs(cint16_t z) {
    /* Cheat for now and use floating point sqrt(). */
    return lrint(sqrt((double)z.real*(double)z.real +
                      (double)z.imag*(double)z.imag));
}

int16_t read_sample(int16_t (*ptrSamplingFunc)(int), int FIFO_index) {
    return ptrSamplingFunc(FIFO_index);
}

int32_t goertzel(double detect_hz, double sample_rate_hz, int N, int16_t (*ptrSamplingFunc)(int), int FIFO_index) {
    /* Notation from p. 710 of Lyons. */

    /* Index of DFT frequency bin to calculate. */
    const double m = (N * detect_hz) / sample_rate_hz;

    /* This complex feedforward coefficient allows a single zero to
       cancel one of the complex poles.  It can be calculated in
       advance. */
    const double complex dbl_coeff_ff = -cexp(-I*2*M_PI*m/N);

    const int coeff_ff_Qn = 15;  /* Q1.15 */
    cint16_t coeff_ff;
    coeff_ff.real = Qn_FROM_DOUBLE(creal(dbl_coeff_ff), coeff_ff_Qn);
    coeff_ff.imag = Qn_FROM_DOUBLE(cimag(dbl_coeff_ff), coeff_ff_Qn);

    /* Feedback coefficient. */
    double dbl_coeff_fb = 2*cos(2*M_PI*m/N);
    const int coeff_fb_Qn = 14;  /* Q2.14 */
    int16_t coeff_fb = Qn_FROM_DOUBLE(dbl_coeff_fb, coeff_fb_Qn);

    const int w_Qn = 15;
    int32_t w[3] = {0};         /* Delay line. Q17.15. */

    int sample_index = 0;

    /* Read and process N+1 samples. */
    while (1) {
        const int x_Qn = 15;  /* Q1.15. */
        int16_t x = read_sample(ptrSamplingFunc, FIFO_index);

        if (sample_index == N) {
            /* The final x input sample should be forced to zero. */
            x = 0;
        }

        /* Manually shift delay line and calculate next value. */
        w[2] = w[1];
        w[1] = w[0];

        /* w[0] = x + (coeff_fb * w[1]) - w[2] */
        w[0] = MUL_Qn_Qn((int64_t)w[1], (int64_t)coeff_fb, w_Qn, coeff_fb_Qn);
        w[0] = ADD_Qn_Qn(w[0], x, w_Qn, x_Qn);
        w[0] = ADD_Qn_Qn(w[0], -w[2], w_Qn, w_Qn);

        if (sample_index++ == N) {
            /* End of Goertzel alogorithm for this buffer. Apply the
             * feedforward coefficient to generate final output. */
            const int y_Qn = 5;
            cint16_t y; /* y = w[0] + coeff_ff * w[1];  complex multiply. */
            y.real = ROUND_OFF_Qn(w[0] +
                                  MUL_Qn_Qn((int64_t)coeff_ff.real, w[1],
                                            coeff_ff_Qn, w_Qn), w_Qn - y_Qn);
            y.imag = ROUND_OFF_Qn(
                MUL_Qn_Qn((int64_t)coeff_ff.imag, w[1],
                          coeff_ff_Qn, w_Qn), w_Qn - y_Qn);

            int32_t dft_mag = cint16_abs(y);

            return dft_mag;
        }
    }
}

// int main(void) {
//     /* Input sample rate. */
//     const double sample_rate_hz = 48000.0;
// 
//     /* Frequency of DFT bin to calculate. */
//     const double detect_hz = 440.0;
// 
//     /* Number of samples for detection.  Need not be a power of 2. */
//     const int N = 1024;
// 
//     while(1) {
//         int32_t dft_mag = goertzel(detect_hz, sample_rate_hz, N);
// 
//         /* Normalize the DFT bin value by N.  A maximum amplitude
//            signal at the detection frequency should give a value
//            of 1/2. */
//         double dft_mag_norm = dft_mag/N;
//         printf("dft_mag: %u, dft_mag_norm: %g\n", dft_mag, dft_mag_norm);
//     }
// }
