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
#include <fstream>
#include <cmath>
#include <string>
#include <cassert>

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

const unsigned int sampleFreq = 32000;
const float secondsPerSample = 1.0 / (float)sampleFreq;
const unsigned int samplesPerSymbol = (60 * sampleFreq) / 2000;
const unsigned int markFreq = 667;
const unsigned int spaceFreq = 600;
const unsigned int S = 34 * 30 * samplesPerSymbol;
static float samples[S];
static unsigned int sampleCount = 0;

static void shift_block(q15* fftBlock, uint16_t fftN, uint16_t blockSize) {
    for (uint16_t i = 0; i < fftN; i++) {
        if (i < (fftN - blockSize)) {
            fftBlock[i] = fftBlock[i + blockSize];
        } else {
            fftBlock[i] = 0;
        }
    }
}

int main(int argc, const char** argv) {
    
    {
        // Load file and normalize
        ifstream infile("demo/AK1WI-SCAMP-40m.txt");
        std::string line;

        sampleCount = 0;
        while (std::getline(infile, line)) {
            float sample = std::stof(line);
            samples[sampleCount++] = sample / 32000.0;
            assert(sampleCount < S);
        }

        /*
        float min = 0, max = 0;
        for (unsigned int i = 0; i < sampleCount; i++) {
            min = std::min(min, samples[i]);
            max = std::min(max, samples[i]);
        }
        cout << "Min " << min << " Max " << max << endl;
        */
    }

    // Now decode
    {
        ClockRecoveryPLL pll(sampleFreq);
        pll.setBitFrequencyHint(33);
        // Size of the FFT
        const uint16_t fftN = 2048;
        // The samples are processed one block at a time. This is the
        // size of the block.
        const uint16_t blockSize = fftN;
        cout << "Samples " << S << endl;
        cout << "Block size " << blockSize << endl;
        // Calculate the duration of the block in seconds
        const float blockDuration = (float)blockSize / (float)sampleFreq;
        cout << "block duration " << blockDuration << endl;
        // Calculate the symbol duration in seconds
        const float symbolDuration = (float)samplesPerSymbol / (float)sampleFreq;
        cout << "symbol duration " << symbolDuration << endl;
        // Calculate the duration of the "long mark"
        const float longMarkDuration = 24.0 * symbolDuration;
        cout << "long mark duration " << longMarkDuration << endl;
        // Calculate the number of blocks that make up the long mark and
        // round down.  We give this a slight haircut to improve robustness.
        const uint16_t longMarkBlocks = ((longMarkDuration / blockDuration) * 0.85);
        
        // This is the pointer into the main datastream
        uint32_t dataPtr = 0;

        q15 window[fftN];
        q15 trigTable[fftN];

        const FixedFFT fft(fftN, trigTable);

        // Build the window (raised cosine)
        for (uint16_t i = 0; i < fftN; i++) {
            window[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * PI * ((float) i) / ((float)fftN))));
        }

        uint16_t blockCount = 0;

        // This is where we store the history of the bin with the largest 
        // magnitude.
        const uint16_t blockHistorySize = longMarkBlocks;
        uint16_t maxBinHistory[blockHistorySize];
        float powerFractionHistory[blockHistorySize];
        for (uint16_t i = 0; i < blockHistorySize; i++) {
            maxBinHistory[i] = 0;
            powerFractionHistory[i] = 0;
        }

        bool locked = false;
        uint16_t lockedBin = 0;

        // Walk through the data one block at a time
        while ((dataPtr + blockSize) < sampleCount) {            

            float timeMarkMs = 1000.0 * ((float)dataPtr * secondsPerSample);

            blockCount++;

            // Do the FFT in a separate buffer, including the window
            cq15 x[fftN];
            for (uint16_t i = 0; i < fftN; i++) {
                q15 sample = f32_to_q15(samples[dataPtr + i]);
                //x[i].r = mult_q15(sample, window[i]);
                x[i].r = sample;
                x[i].i = 0;
            }
            fft.transform(x);

            // Find the largest power. Notice that we ignore bin 0 (DC)
            // since that's not relevant.
            const unsigned int b = max_idx(x, 1, (fftN / 2) - 1);
            const float b_power = x[b].mag_f32_squared();
            const unsigned int b_hz = (sampleFreq / fftN) * b;
            cout << blockCount << " " << timeMarkMs << " " << b << " " << b_hz << " " << b_power << endl;

            // If we are not yet locked, try to lock
            if (!locked) {

                // Find the total power
                float totalPower = 0;
                for (uint16_t i = 0; i < fftN / 2; i++) {
                    totalPower += x[i].mag_f32_squared();
                }
                // Find the percentage of power at the max (and adjacent)
                float maxBinPower = x[b].mag_f32_squared();
                if (b > 1) {
                    maxBinPower += x[b - 1].mag_f32_squared();
                }
                if (b < (fftN / 2) - 1) {
                    maxBinPower += x[b + 1].mag_f32_squared();
                }
                float powerFract = maxBinPower / totalPower;

                // Shift the history collection area and accumulate the new 
                // observation.
                for (uint16_t i = 0; i < blockHistorySize - 1; i++) {
                    maxBinHistory[i] = maxBinHistory[i + 1];
                    powerFractionHistory[i] = powerFractionHistory[i + 1];
                }
                maxBinHistory[blockHistorySize - 1] = b;
                powerFractionHistory[blockHistorySize - 1] = powerFract;

                // Find the average and the max variance 
                uint32_t t = 0;
                float p = 0;
                for (uint16_t i = 0; i < blockHistorySize; i++) {
                t += maxBinHistory[i];
                p += powerFractionHistory[i];
                }            
                uint16_t averageMaxBin = t / blockHistorySize;
                float averagePowerFraction = p / (float)blockHistorySize;
                //cout << averageMaxBin << " " << averagePowerFraction << endl;

                uint16_t maxDiff = 0;
                for (uint16_t i = 0; i < blockHistorySize; i++) {
                    uint16_t absDiff = 
                        std::abs((int16_t)maxBinHistory[i] - (int16_t)averageMaxBin);
                    if (absDiff > maxDiff) {
                        maxDiff = absDiff;
                    }
                }

                // If the variance is small and the power is large then 
                // assume we're seeing the "long mark"                
                if (maxDiff <= 2 && averagePowerFraction > 0.40) {
                    locked = true;
                    lockedBin = b;
                    cout << "LOCKED ON BIN " << lockedBin << endl;
                }
            }

            dataPtr += blockSize;
        }

        cout << endl;
    }

    return 0;
}
