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
#include <cmath>
#include <random>

#include "Util.h"
#include "fixed_fft.h"

/*
Max idx 85
Max mag 0.158523
Max frq 680
M-1 mag 0.0261144
M+1 mag 0.0145446
*/

using namespace std;
using namespace scamp;

static const float PI = 3.1415926f;

/**
 * Generates a tone with optional noise.  The resulting signal will
 * be re-normalized back to the amplitude provided.
 */
// void make_tone(q15* output, 
//     const unsigned int len, float sample_freq_hz, 
//     float tone_freq_hz, float amplitude, float noise_amplitude) {

//     static std::random_device rd{};
//     static std::mt19937 gen{rd()}; 

//     // values near the mean are the most likely
//     // standard deviation affects the dispersion of generated values from the mean
//     std::normal_distribution<float> d(0.0, 1.0);

//     float phi = 0;
//     float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
//     float max_amp = 0;

//     // Build tone in float space
//     float tone[len];
//     float max_sig = 0;
//     for (unsigned int i = 0; i < len; i++) {
//         float sig = std::cos(phi) * amplitude + d(gen) * noise_amplitude;
//         if (std::abs(sig) > max_sig) {
//             max_sig = std::abs(sig);
//         }
//         tone[i] = sig;
//         phi += omega;
//     }

//     // Normalize back to the tone amplitude and convert
//     for (unsigned int i = 0; i < len; i++) {
//         float n_sig = (tone[i] / max_sig) * amplitude;
//         output[i] = f32_to_q15(n_sig);
//     }
// }

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

float ang(q15 r, q15 i) {
    float ref = q15_to_f32(r);
    float imf = q15_to_f32(i);
    return std::atan2(ref, imf);
}

/**
 * data[i] = data[i] * window[i]
*/
void times_equal(q15* data, q15* window, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        q15 d = mult_q15(data[i], window[i]);
        data[i] = d;
    }
}

float corr(q15* data, q15* carrier, uint16_t len) {
    float result = 0;
    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(data[i]);
        float b = q15_to_f32(carrier[i]);
        result += a * b;
    }
    return result / (float)len;
}

int main(int argc, const char** argv) {

    float tone_freq_hz = 670.0;
    float detected_freq_hz = 0;
    float sample_freq_hz = 2000.0;
    const unsigned int N = 1024;
    q15 sample_r[N];

   {
        // Make a tone and then perform the FFT
        float amplitude = 1.0;
        make_tone(sample_r, N, sample_freq_hz, tone_freq_hz, amplitude);

        // Build the window (raised cosine)
        q15 hann_window[N];
        for (unsigned int i = 0; i < N; i++) {
            hann_window[i] = f32_to_q15(0.5 * (1.0 - cos(2.0 * PI * ((float) i) / ((float)N))));
        }

        //times_equal(sample_r, hann_window, N);
        
        FixedFFT<N> fft;

        // Make a complex series
        cq15 sample[N];
        for (uint16_t i = 0; i < N; i++) {
            sample[i].r = sample_r[i];
            sample[i].i = 0;
        }

        fft.transform(sample);

        unsigned int b = max_idx(sample, 1, (N / 2) - 1);
        cout << "Max idx " << b << endl;
        cout << "Max mag " << sample[b].mag_f32() << endl;
        cout << "Max frq " << fft.binToFreq(b, sample_freq_hz) << endl;
        cout << "M-1 mag " << sample[b - 1].mag_f32() << endl;
        cout << "M+1 mag " << sample[b + 1].mag_f32() << endl;

        detected_freq_hz = fft.binToFreq(b, sample_freq_hz);
   }

    // Correlation test using the frequency we detected before
    {
        const unsigned int blockN = 64;

        // Sample a block from the original tone
        cq15 sig_sample[blockN];
        for (uint16_t i = 0; i < blockN; i++) {
            sig_sample[i].r = sample_r[i];
            sig_sample[i].i = 0;
        }

        // This is the center frequency we detected.  Tune the LO accordingly,
        // but put it out of phase to show that the quadrature demodulator is
        // working.
        float lo_freq_hz = detected_freq_hz;
        float lo_amplitude = 1.0;
        float lo_phase = 90;
        cq15 lo_sample[blockN];

        make_complex_tone(lo_sample, blockN, sample_freq_hz, lo_freq_hz, lo_amplitude);
        cout << "Center          " << complex_corr(sig_sample, lo_sample, blockN) << endl;

        make_complex_tone(lo_sample, blockN, sample_freq_hz, lo_freq_hz, lo_amplitude, lo_phase);
        cout << "Center (Phased) " << complex_corr(sig_sample, lo_sample, blockN) << endl;

        make_complex_tone(lo_sample, blockN, sample_freq_hz, lo_freq_hz - 25, lo_amplitude);
        cout << "Center - 25     " << complex_corr(sig_sample, lo_sample, blockN) << endl;

        make_complex_tone(lo_sample, blockN, sample_freq_hz, lo_freq_hz + 25, lo_amplitude);
        cout << "Center + 25     " << complex_corr(sig_sample, lo_sample, blockN) << endl;
    }

    return 0;
}
