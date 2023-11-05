#include <iostream>
#include <cmath>

#include "fixed_fft.h"

using namespace std;

const float PI = 3.1415926f;

void make_tone(q15* output_r, q15* output_i, 
  unsigned int len, float sample_freq_hz, 
  float tone_freq_hz, float amplitude) {
  float phi = 0;
  float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
  for (unsigned int i = 0; i < len; i++) {
    float sig = std::cos(phi) * amplitude;
    output_r[i] = float2q15(sig);
    output_i[i] = 0;
    phi += omega;
  }
}

float mag(q15 r, q15 i) {
    float ref = q152float(r);
    float imf = q152float(i);
    return std::sqrt(ref * ref + imf * imf);
}

float ang(q15 r, q15 i) {
    float ref = q152float(r);
    float imf = q152float(i);
    return std::atan2(ref, imf);
}

unsigned int max_bin(q15* sample_r, q15* sample_i, unsigned int len) {
    float max_mag = 0;
    unsigned int max_bin = 0;
    for (unsigned int i = 0; i < len; i++) {
        float m = mag(sample_r[i], sample_i[i]);
        if (m > max_mag) {
            max_mag = m;
            max_bin = i;
        }
    }
    return max_bin;
}

/**
 * data[i] = data[i] * window[i]
*/
void times_equal(q15* data, q15* window, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        q15 d = multq15(data[i], window[i]);
        data[i] = d;
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
    //const unsigned int len = 1024;
    //const unsigned int len = 512;
    const unsigned int len = 256;
    q15 sample_r[len];
    q15 sample_i[len];
    float sample_freq_hz = 2048.0;
    float tone_freq_hz = 670.0;
    float amplitude = 1.0;

    // Build the window (raised cosine)
    q15 hann_window[len];
    for (unsigned int i = 0; i < len; i++) {
        hann_window[i] = float2q15(0.5 * (1.0 - cos(6.283 * ((float) i) / ((float)len))));
    }

    make_tone(sample_r, sample_i, len, sample_freq_hz, tone_freq_hz, amplitude);
    times_equal(sample_r, hann_window, len);

    //cout << "Time Domain:" << endl;
    //for (unsigned int i = 0; i < len; i++) {
        //cout << i << " " << mag(sample_r[i], sample_i[i]) << endl;
        //cout << i << " " << fix2float15(sample_r[i]) << endl;
    //}
    
    fixed_fft_setup();
    fixed_fft(sample_r, sample_i);

    //cout << "Freq Domain:" << endl;
    //for (unsigned int i = 0; i < len; i++) {
    //    cout << i << " " << mag(sample_r[i], sample_i[i]) << endl;
    //    cout << i << " " << fix2float15(sample_r[i]) << endl;
    //}

    unsigned int b = max_bin(sample_r, sample_i, len / 2);
    cout << "Max idx " << b << endl;
    cout << "Max mag " << mag(sample_r[b], sample_i[b]) << endl;
    cout << "Max frq " << ((float)b / (float)len) * (sample_freq_hz) << endl;

    //cout << "M-2 mag "<< mag(sample_r[b-2], sample_i[b-2]) << endl;
    cout << "M-1 mag "<< mag(sample_r[b-1], sample_i[b-1]) << endl;
    cout << "M+1 mag "<< mag(sample_r[b+1], sample_i[b+1]) << endl;
    //cout << "M+2 mag "<< mag(sample_r[b+2], sample_i[b+2]) << endl;

    return 0;
}


