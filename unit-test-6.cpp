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

void make_tone(complex_q15* output, 
  unsigned int len, float sample_freq_hz, 
  float tone_freq_hz, float amplitude) {
  float phi = 0;
  float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
  for (unsigned int i = 0; i < len; i++) {
    float sig = std::cos(phi) * amplitude;
    output[i].r = f32_to_q15(sig);
    output[i].i = 0;
    phi += omega;
  }
}

float ang(q15 r, q15 i) {
    float ref = q15_to_f32(r);
    float imf = q15_to_f32(i);
    return std::atan2(ref, imf);
}

unsigned int max_bin(complex_q15* sample, unsigned int len) {
    float max_mag = 0;
    unsigned int max_bin = 0;
    for (unsigned int i = 0; i < len; i++) {
        float m = sample[i].mag_f32();
        if (m > max_mag) {
            max_mag = m;
            max_bin = i;
        }
    }
    return max_bin;
}

/**
 * data[i].r = data[i].r * window[i]
*/
void times_equal(complex_q15* data, q15* window, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        q15 d = mult_q15(data[i].r, window[i]);
        data[i].r = d;
    }
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

    // Make a tone and then perform the FFT
    const unsigned int N = 1024;
    complex_q15 sample[N];
    //q15 sample_i[N];
    float sample_freq_hz = 2048.0;
    float tone_freq_hz = 670.0;
    float amplitude = 1.0;

    // Build the window (raised cosine)
    q15 hann_window[N];
    for (unsigned int i = 0; i < N; i++) {
        hann_window[i] = f32_to_q15(0.5 * (1.0 - cos(6.283 * ((float) i) / ((float)N))));
    }

    make_tone(sample, N, sample_freq_hz, tone_freq_hz, amplitude);
    times_equal(sample, hann_window, N);

    //cout << "Time Domain:" << endl;
    //for (unsigned int i = 0; i < len; i++) {
        //cout << i << " " << mag(sample_r[i], sample_i[i]) << endl;
        //cout << i << " " << fix2float15(sample_r[i]) << endl;
    //}
    
    FixedFFT<N> fft;

    fft.transform(sample);

    //cout << "Freq Domain:" << endl;
    //for (unsigned int i = 0; i < len; i++) {
    //    cout << i << " " << mag(sample_r[i], sample_i[i]) << endl;
    //    cout << i << " " << fix2float15(sample_r[i]) << endl;
    //}

    unsigned int b = max_bin(sample, N / 2);
    cout << "Max idx " << b << endl;
    cout << "Max mag " << sample[b].mag_f32() << endl;
    cout << "Max frq " << fft.binToFreq(b, sample_freq_hz) << endl;

    cout << "M-1 mag " << sample[b-1].mag_f32() << endl;
    cout << "M+1 mag " << sample[b+1].mag_f32() << endl;

    return 0;
}
