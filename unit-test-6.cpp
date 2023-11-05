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

#include "fixed_fft.h"

/*
Max idx 85
Max mag 0.158523
Max frq 680
M-1 mag 0.0261144
M+1 mag 0.0145446
*/

using namespace std;

const float PI = 3.1415926f;

/**
 * Generates a tone with optional noise.  The resulting signal will
 * be re-normalized back to the amplitude provided.
 */
void make_tone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float noise_amplitude) {

    static std::random_device rd{};
    static std::mt19937 gen{rd()}; 

    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<float> d(0.0, 1.0);

    float phi = 0;
    float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
    float max_amp = 0;

    // Build tone in float space
    float tone[len];
    float max_sig = 0;
    for (unsigned int i = 0; i < len; i++) {
        float sig = std::cos(phi) * amplitude + d(gen) * noise_amplitude;
        if (std::abs(sig) > max_sig) {
            max_sig = std::abs(sig);
        }
        tone[i] = sig;
        phi += omega;
    }

    // Normalize back to the tone amplitude and convert
    for (unsigned int i = 0; i < len; i++) {
        float n_sig = (tone[i] / max_sig) * amplitude;
        output[i] = f32_to_q15(n_sig);
    }
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
    /*
    float ra = 0.7071;
    float ia = 0.7071;
    cout << "Mag Q1 " << mag(float2fix15(ra), float2fix15(ia)) << endl;
    cout << "Ang Q1 " << ang(float2fix15(ra), float2fix15(ia)) << endl;
    cout << "Mag Q2 " << mag(float2fix15(-ra), float2fix15(ia)) << endl;
    cout << "Ang Q2 " << ang(float2fix15(-ra), float2fix15(ia)) << endl;
    cout << "Mag Q3 " << mag(float2fix15(-ra), float2fix15(-ia)) << endl;
    cout << "Mag Q4 " << mag(float2fix15(ra), float2fix15(-ia)) << endl;
    */

   float tone_freq_hz = 670.0;
   float detected_freq_hz = 0;
   float sample_freq_hz = 2000.0;

   {
        // Make a tone and then perform the FFT
        const unsigned int N = 1024;
        q15 sample_r[N];
        float amplitude = 1.0;

        // Build the window (raised cosine)
        q15 hann_window[N];
        for (unsigned int i = 0; i < N; i++) {
            hann_window[i] = f32_to_q15(0.5 * (1.0 - cos(2.0 * PI * ((float) i) / ((float)N))));
        }

        make_tone(sample_r, N, sample_freq_hz, tone_freq_hz, amplitude, amplitude / 10.0);
        times_equal(sample_r, hann_window, N);
        
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

        cout << "M-1 mag " << sample[b-1].mag_f32() << endl;
        cout << "M+1 mag " << sample[b+1].mag_f32() << endl;

        detected_freq_hz = fft.binToFreq(b, sample_freq_hz);
   }

    // Correlation test using the frequency we detected before
    {
        const unsigned int N = 64;

        // This is the center frequency we detected
        float carrier_freq_hz = detected_freq_hz;
        float carrier_amplitude = 1.0;
        q15 carrier_sample[N];
        make_tone(carrier_sample, N, sample_freq_hz, carrier_freq_hz, carrier_amplitude, 0);

        // Check to see how fast the correlation rolls off
        float sig_amplitude = 1.0;
        q15 sig_sample[N];

        float sig_freq_hz = tone_freq_hz;
        make_tone(sig_sample, N, sample_freq_hz, sig_freq_hz, sig_amplitude, sig_amplitude / 10.0);
        cout << "Center          " << corr(sig_sample, carrier_sample, N) << endl;
        cout << "Center      Q15 " << q15_to_f32(corr_q15(sig_sample, carrier_sample, N)) << endl;

        sig_freq_hz = tone_freq_hz - 50;
        make_tone(sig_sample, N, sample_freq_hz, sig_freq_hz, sig_amplitude, sig_amplitude / 10.0);
        cout << "Center - 50     " << corr(sig_sample, carrier_sample, N) << endl;
        cout << "Center - 50 Q15 " << q15_to_f32(corr_q15(sig_sample, carrier_sample, N)) << endl;

        sig_freq_hz = tone_freq_hz - 25;
        make_tone(sig_sample, N, sample_freq_hz, sig_freq_hz, sig_amplitude, sig_amplitude / 10.0);
        cout << "Center - 25     " << corr(sig_sample, carrier_sample, N) << endl;
        cout << "Center - 25 Q15 " << q15_to_f32(corr_q15(sig_sample, carrier_sample, N)) << endl;

        sig_freq_hz = tone_freq_hz + 25;
        make_tone(sig_sample, N, sample_freq_hz, sig_freq_hz, sig_amplitude, sig_amplitude / 10.0);
        cout << "Center + 25     " << corr(sig_sample, carrier_sample, N) << endl;
        cout << "Center + 25 Q15 " << q15_to_f32(corr_q15(sig_sample, carrier_sample, N)) << endl;

        sig_freq_hz = tone_freq_hz + 50;
        make_tone(sig_sample, N, sample_freq_hz, sig_freq_hz, sig_amplitude, sig_amplitude / 10.0);
        cout << "Center + 50     " << corr(sig_sample, carrier_sample, N) << endl;
        cout << "Center + 50 Q15 " << q15_to_f32(corr_q15(sig_sample, carrier_sample, N)) << endl;
    }

    return 0;
}
