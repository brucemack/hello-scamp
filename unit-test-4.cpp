/*
SCAMP Encoder/Decoder
Copyright (C) 2023 - Bruce MacKinnon KC1FSZ

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <iostream>
#include "Util.h"
#include "fixed_math.h"

using namespace std;
using namespace scamp;

float corr(q15* data, q15* carrier, uint16_t len) {
    float result = 0;
    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(data[i]);
        float b = q15_to_f32(carrier[i]);
        result += a * b;
    }
    return result / (float)len;
}

float complex_corr(cq15* c0, cq15* c1, uint16_t len) {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(c0[i].r);
        float b = q15_to_f32(c0[i].i);
        float c = q15_to_f32(c1[i].r);
        // Complex conjugate
        float d = -q15_to_f32(c1[i].i);
        // Use the method that minimizes multiplication
        float ac = a * c;
        float bd = b * d;
        float a_plus_b = a + b;
        float c_plus_d = c + d;
        float p0 = a_plus_b * c_plus_d;
        result_r += (ac - bd) / (float)len;
        result_i += (p0 - ac - bd) / (float)len;
    }

    return std::sqrt(result_r * result_r + result_i * result_i);
}

int main(int argc, const char** argv) {

    const uint16_t N = 1024;
    float sample_freq_hz = 2000.0;
    float tone_freq_hz = 670.0;
    float tone_amp = 1.0;
    float lo_freq_hz = 670.0;
    float lo_amp = 1.0;

    // Demonstrate correlation in phase (real only)
    // Here you can see the correlation is almost 0.5 (highest possible)
    {
        q15 tone_sample_r[N];
        make_tone(tone_sample_r, N, sample_freq_hz, tone_freq_hz, tone_amp);
        q15 lo_sample_r[N];
        make_tone(lo_sample_r, N, sample_freq_hz, lo_freq_hz, lo_amp);
        cout << "(Real) In phase " << corr(tone_sample_r, lo_sample_r, N) << endl;
    }

    // Demonstrate correlation out of phase (real only).
    // Here you can see that the correlation goes to zero even though
    // the frequency is exactly right.
    {
        q15 tone_sample_r[N];
        make_tone(tone_sample_r, N, sample_freq_hz, tone_freq_hz, tone_amp, 90.0);
        q15 lo_sample_r[N];
        make_tone(lo_sample_r, N, sample_freq_hz, lo_freq_hz, lo_amp);
        cout << "(Real) Out of phase " << corr(tone_sample_r, lo_sample_r, N) << endl;
    }

    // Demonstrate correlation in phase (complex)
    // Notice that the correlation is around 1.0
    {
        cq15 tone_sample[N];
        make_complex_tone(tone_sample, N, sample_freq_hz, tone_freq_hz, tone_amp);
        cq15 lo_sample[N];
        make_complex_tone(lo_sample, N, sample_freq_hz, lo_freq_hz, lo_amp);
        cout << "(Complex) In phase " << complex_corr(tone_sample, lo_sample, N) << endl;
    }

    // Demonstrate complex correlation in phase but off frequency
    {
        cq15 tone_sample[N];
        make_complex_tone(tone_sample, N, sample_freq_hz, tone_freq_hz, tone_amp);
        cq15 lo_sample[N];
        make_complex_tone(lo_sample, N, sample_freq_hz, lo_freq_hz - 10, lo_amp);
        cout << "(Complex) Off freq by 10 Hz " << complex_corr(tone_sample, lo_sample, N) << endl;
    }

    // Demonstrate complex correlation in phase but off frequency.
    // Notice that the correlation is still 1.0
    {
        cq15 tone_sample[N];
        make_complex_tone(tone_sample, N, sample_freq_hz, tone_freq_hz, tone_amp, 90);
        cq15 lo_sample[N];
        make_complex_tone(lo_sample, N, sample_freq_hz, lo_freq_hz, lo_amp);
        cout << "(Complex) Out of phase " << complex_corr(tone_sample, lo_sample, N) << endl;
    }

    // Demonstrate complex correlation in phase but off frequency.
    // Notice that the correlation is still 1.0
    {
        cq15 tone_sample[N];
        make_complex_tone(tone_sample, N, sample_freq_hz, tone_freq_hz, tone_amp, 180);
        cq15 lo_sample[N];
        make_complex_tone(lo_sample, N, sample_freq_hz, lo_freq_hz, lo_amp);
        cout << "(Complex) Out of phase " << complex_corr(tone_sample, lo_sample, N) << endl;
    }

    return 0;
}
