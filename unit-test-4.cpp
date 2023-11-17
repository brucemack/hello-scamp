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
#include "fixed_fft.h"
#include "TestModem2.h"

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

int main(int argc, const char** argv) {

    const uint16_t N = 1024;
    const uint16_t fftN = 512;
    float sample_freq_hz = 2000.0;
    float tone_freq_hz = 670.0;
    float tone_amp = 1.0;
    float lo_freq_hz = 670.0;
    float lo_amp = 1.0;

    // Make a tone and then apply the FFT
    {
        float samples[512];
        TestModem2 modem(samples, 512, sample_freq_hz, 256, tone_freq_hz, tone_freq_hz,
            0.5, 0.1);
        //modem.sendMark();
        modem.sendSilence();
        modem.sendMark();

        // Space for the demodulator to work in (no dynamic memory allocation!)
        q15 trigTable[fftN];
        q15 window[fftN];
        q15 buffer[fftN];
        cq15 fftResult[fftN];
        FixedFFT fft(fftN, trigTable);

        cq15 x[fftN];
        // Do the FFT in the result buffer, including the window.  
        for (uint16_t i = 0; i < fftN; i++) {
            x[i].r = f32_to_q15(samples[i]);
            x[i].i = 0;
        }

        fft.transform(x);
        
        //render_spectrum(cout, x, fftN, sample_freq_hz);
    }

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

    // Correlate real with complex
    {
        float samples[512];
        //
        TestModem2 modem(samples, 512, sample_freq_hz, 512, tone_freq_hz, tone_freq_hz,
            0.5, 0.05);
        modem.sendMark();
        
        uint16_t bin = (tone_freq_hz * 512) / sample_freq_hz;
        cout << "Bin " << bin << endl;
        q15 signal[512];
        for (uint16_t i = 0; i < 512; i++) {
            signal[i] = f32_to_q15(samples[i]);
        }

        {
            cq15 lo_sample[512];
            make_complex_tone_2(lo_sample, 512, bin - 2, 512, 0.5);
            float corr = complex_corr_2(signal, 0, 512, lo_sample, 512); 
            cout << "CORR -2 " << corr << endl;
        }
        {
            cq15 lo_sample[512];
            make_complex_tone_2(lo_sample, 512, bin - 1, 512, 0.5);
            float corr = complex_corr_2(signal, 0, 512, lo_sample, 512); 
            cout << "CORR -1 " << corr << endl;
        }
        {
            cq15 lo_sample[512];
            make_complex_tone_2(lo_sample, 512, bin, 512, 0.5);
            float corr = complex_corr_2(signal, 0, 512, lo_sample, 512); 
            cout << "CORR    " << corr << endl;
        }
        {
            cq15 lo_sample[512];
            make_complex_tone_2(lo_sample, 512, bin + 1, 512, 0.5);
            float corr = complex_corr_2(signal, 0, 512, lo_sample, 512); 
            cout << "CORR +1 " << corr << endl;
        }
        {
            cq15 lo_sample[512];
            make_complex_tone_2(lo_sample, 512, bin + 2, 512, 0.5);
            float corr = complex_corr_2(signal, 0, 512, lo_sample, 512); 
            cout << "CORR +2 " << corr << endl;
        }
    }

    return 0;
}
