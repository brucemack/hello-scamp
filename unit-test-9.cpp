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

==============================================================================
Tests related to down-sampling.
*/
#include <fstream>
#include <cmath>
#include <string>
#include <cstdint>
#include <iomanip>

#include "Util.h"
#include "Symbol6.h"
#include "CodeWord24.h"
#include "Frame30.h"
#include "FileModulator.h"
#include "TestModem.h"
#include "TestModem2.h"
#include "FileModulator.h"
#include "ClockRecoveryPLL.h"
#include "fixed_math.h"
#include "fixed_fft.h"

using namespace std;
using namespace scamp;

const float PI = 3.1415926f;

const unsigned int sampleCount = 16 * 1024;
static q15 samples[sampleCount];
static q15 filteredSamples[sampleCount];

int main(int argc, const char** argv) {

    // FFT
    uint16_t fftN = 512;
    q15 window[fftN];
    q15 trigTable[fftN];

    const FixedFFT fft(fftN, trigTable);

    // Build the window (raised cosine)
    for (uint16_t i = 0; i < fftN; i++) {
        window[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * PI * ((float) i) / ((float)fftN))));
    }

    unsigned int origSampleFreq = 2000;

    {
        unsigned int toneFreq0 = 300;
        unsigned int toneFreq1 = 800;

        // Fill buffer with two tones
        make_tone(samples, sampleCount, origSampleFreq, toneFreq0, 0.5);
        addTone(samples, sampleCount, origSampleFreq, toneFreq1, 0.5);

        // Do the FFT in a separate buffer, including the window
        cq15 x[fftN];
        for (uint16_t i = 0; i < fftN; i++) {
            x[i].r = mult_q15(samples[i], window[i]);
            //x[i].r = samples[i];
            x[i].i = 0;
        }
        fft.transform(x);

        // Find the largest power. Notice that we ignore bin 0 (DC)
        // since that's not relevant.
        const uint16_t b = max_idx(x, 1, (fftN / 2) - 1);
        const float b_max_power = x[b].mag_f32_squared();
        const unsigned int b_max_hz = fft.binToFreq(b, origSampleFreq);
        cout << b << " " << b_max_hz << " " << b_max_power << endl;

        // Display a spectrum
        //render_spectrum(cout, x, fftN, origSampleFreq);
    }

    // ------ Attempt to Low-Pass Filter --------------------------------------------
    {
        // Low-pass filter with cut-off 550, transition 200

        float h[] = {
            0.000000000000000000,
            0.000007557960334710,
            -0.000103224320123774,
            0.000000000000000001,
            0.000489716681900860,
            -0.000264603155652994,
            -0.001222293994402640,
            0.001226311088660148,
            0.002158267787169483,
            -0.003504168412246353,
            -0.002722302418093459,
            0.007735421573498187,
            0.001696950098657745,
            -0.014271549283750381,
            0.002906824067285976,
            0.022852455739101822,
            -0.014070165120926129,
            -0.032439640479462260,
            0.037099215119700719,
            0.041349585583744575,
            -0.088225036644122165,
            -0.047697200474843847,
            0.311996135778338624,
            0.550003485650462309,
            0.311996135778338679,
            -0.047697200474843847,
            -0.088225036644122151,
            0.041349585583744589,
            0.037099215119700726,
            -0.032439640479462260,
            -0.014070165120926129,
            0.022852455739101843,
            0.002906824067285976,
            -0.014271549283750377,
            0.001696950098657743,
            0.007735421573498189,
            -0.002722302418093461,
            -0.003504168412246354,
            0.002158267787169486,
            0.001226311088660149,
            -0.001222293994402640,
            -0.000264603155652995,
            0.000489716681900860,
            0.000000000000000001,
            -0.000103224320123774,
            0.000007557960334711,
            0.000000000000000000 
        };

        // Apply the filter to the data
        for (uint16_t n = 0; n < sampleCount; n++) {
            filteredSamples[n] = 0;
            for (uint16_t k = 0; k < 47; k++) {
                int16_t n_minus_k = n - k;
                if (n_minus_k < 0) {
                    n_minus_k = 0;
                }
                q15 h0 = f32_to_q15((h[k]));
                filteredSamples[n] += mult_q15(h0, samples[n_minus_k]);
            }
        }

        // FFT with window
        cq15 x[fftN];
        for (uint16_t i = 0; i < fftN; i++) {
            x[i].r = mult_q15(filteredSamples[i], window[i]);
            //x[i].r = samples[i];
            x[i].i = 0;
        }
        fft.transform(x);

        // Find the largest power. Notice that we ignore bin 0 (DC)
        // since that's not relevant.
        const uint16_t b = max_idx(x, 1, (fftN / 2) - 1);
        const float b_max_power = x[b].mag_f32_squared();
        const unsigned int b_max_hz = fft.binToFreq(b, origSampleFreq);
        cout << b << " " << b_max_hz << " " << b_max_power << endl;

        // Display a spectrum
        render_spectrum(cout, x, fftN, origSampleFreq);
    }
}
