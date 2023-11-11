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

// This shifts an array down towards the lower indices by one block
// size.
static void shift_block_down(q15* d, uint16_t n, uint16_t blockSize) {
    for (uint16_t i = 0; i < n; i++) {
        if (i < (n - blockSize)) {
            d[i] = d[i + blockSize];
        } else {
            d[i] = 0;
        }
    }
}

// ------ Data Area -------

const unsigned int sampleFreq = 2000;
const unsigned int samplesPerSymbol = 60;
const unsigned int markFreq = 667;
const unsigned int spaceFreq = 600;
const unsigned int S = 34 * 30 * samplesPerSymbol;
static float samples[S];

int main(int argc, const char** argv) {

    TestModem2 modem2(samples, S, sampleFreq, samplesPerSymbol, markFreq, spaceFreq);
    
    // =========================================================================
    // Make a message, encode it, and then recover it. In this test we are 
    // using the SCAMP FSK (33.3 bits per second) format.  
    {        
        const char* testMessage = "DE KC1FSZ, GOOD MORNING";

        // Encode the message.  This leads to about 25K samples.
        Frame30 frames[32];
        unsigned int frameCount = encodeString(testMessage, frames, 32, true);
    
        {
            // This is SCAMP FSK
            std::ofstream outfile("bin/scamp-fsk-demo-3.txt");
            FileModulator modem(outfile, sampleFreq, samplesPerSymbol, markFreq, spaceFreq);

            // Silence
            for (unsigned int i = 0; i < 30; i++)
                modem.sendSilence();
            // Transmit a legit message
            for (unsigned int i = 0; i < frameCount; i++) {
                frames[i].transmit(modem);
            }
            // Silence
            for (unsigned int i = 0; i < 30; i++)
                modem.sendSilence();
        }
        
        // We purposely offset the data stream by a half symbol 
        // to stress the PLL.
        //modem.sendHalfSilence();
        // This silence is 30 symbols, or 30 * 60 = 180 samples long
        for (unsigned int i = 0; i < 30; i++)
            modem2.sendSilence();
        // Transmit a legit message
        for (unsigned int i = 0; i < frameCount; i++) {
            frames[i].transmit(modem2);
        }
        for (unsigned int i = 0; i < 30; i++) {
            modem2.sendSilence();
        }
    }
    
    // Now decode
    {
        ClockRecoveryPLL pll(sampleFreq);
        // Purposely set for the wrong frequency to watch the clock 
        // recovery work. But it's close.
        pll.setBitFrequencyHint(36);
        // The samples are processed one block at a time. This is the
        // size of the block.
        const uint16_t blockSize = 32;
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
        const uint16_t longMarkBlocks = ((longMarkDuration / blockDuration) * 0.70);
       
        // This is the pointer into the main data stream
        uint32_t dataPtr = 0;

        const uint16_t fftN = 512;
        q15 fftBlock[fftN];
        q15 window[fftN];
        q15 trigTable[fftN];

        const FixedFFT fft(fftN, trigTable);

        for (uint16_t i = 0; i < fftN; i++) {
            fftBlock[i] = 0;
            // Build the window (raised cosine)
            window[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * PI * ((float) i) / ((float)fftN))));
        }

        uint16_t blockCount = 0;

        // This is where we store the history of the bin with the largest 
        // magnitude.
        const uint16_t blockHistorySize = longMarkBlocks;
        uint16_t maxBinHistory[blockHistorySize];
        for (uint16_t i = 0; i < blockHistorySize; i++) {
            maxBinHistory[i] = 0;
        }

        bool locked = false;
        uint16_t lockedBinMark = 0;

        const uint16_t detectorLength = blockSize;
        cq15 markDetectorTone[detectorLength];
        cq15 spaceDetectorTone[detectorLength];

        // Walk through the data one block at a time
        while ((dataPtr + blockSize) < modem2.getSamplesUsed()) {            

            // Shift the FFT block down (towards the lower indices)
            shift_block_down(fftBlock, fftN, blockSize);

            // Fill in the top part of the FFT block with the new data
            for (uint16_t i = 0; i < blockSize; i++) {
                fftBlock[fftN - blockSize + i] = f32_to_q15(samples[dataPtr + i]);
            }

            // Do the FFT in a separate buffer, including the window
            cq15 x[fftN];
            for (uint16_t i = 0; i < fftN; i++) {
                x[i].r = mult_q15(fftBlock[i], window[i]);
                //x[i].r = fftBlock[i];
                x[i].i = 0;
            }
            fft.transform(x);

            // Find the largest power. Notice that we ignore bin 0 (DC)
            // since that's not relevant.
            const uint16_t maxBin = max_idx(x, 1, fftN / 2);
            const unsigned int maxFreq = fft.binToFreq(maxBin, sampleFreq);

            // If we are not yet locked, try to lock
            if (!locked) {

                // Find the total power
                float totalPower = 0;
                for (uint16_t i = 0; i < fftN / 2; i++) {
                    totalPower += x[i].mag_f32_squared();
                }
                // Find the percentage of power at the max (and two adjacent)
                float maxBinPower = x[maxBin].mag_f32_squared();
                if (maxBin > 1) {
                    maxBinPower += x[maxBin - 1].mag_f32_squared();
                }
                if (maxBin < (fftN / 2) - 1) {
                    maxBinPower += x[maxBin + 1].mag_f32_squared();
                }
                const float maxBinPowerFract = maxBinPower / totalPower;

                // Shift the history collection area and accumulate the new 
                // observation.
                for (uint16_t i = 0; i < blockHistorySize - 1; i++) {
                    maxBinHistory[i] = maxBinHistory[i + 1];
                }
                maxBinHistory[blockHistorySize - 1] = maxBin;

                // Find the variance of the max bin across the recent observations
                // to see if we have stability.  We are only looking 
                float maxBinMean = 0;
                for (uint16_t i = 0; i < blockHistorySize; i++) {
                    maxBinMean += maxBinHistory[i];
                }

                // The locking logic only works when the history is full
                if (blockCount >= blockHistorySize) {
                 
                    maxBinMean /= (float)blockHistorySize;

                    float maxBinVar = 0;
                    for (uint16_t i = 0; i < blockHistorySize; i++) {
                        if (maxBinHistory[i] != 0) {
                            maxBinVar += std::pow(maxBinHistory[i] - maxBinMean, 2);
                        }
                    }
                    maxBinVar /= ((float)blockHistorySize - 1);
                    float maxBinStd = std::sqrt(maxBinVar);
        
                    //cout << blockCount << " " << maxBin << " " << maxBinStd << " " << maxBinPowerFract << endl;

                    // If the standard deviation is small and the power is large then 
                    // assume we're seeing the "long mark"                
                    if (maxBinStd <= 1.0 && maxBinPowerFract > 0.40) {
                        locked = true;
                        lockedBinMark = maxBin;
                        float lockedBinSpread = ((float)(markFreq - spaceFreq) / (float)sampleFreq) * 
                            (float)fftN;
                        float lockedBinSpace = ((float)lockedBinMark - lockedBinSpread);

                        cout << "LOCKED ON BINS " << lockedBinMark << "/" << lockedBinSpace << endl;

                        // Build the tone needed by the quadrature decoder
                        float binScale = (float)detectorLength / (float)fftN;
                        make_complex_tone_2(markDetectorTone, detectorLength, 
                            (float)lockedBinMark * binScale, detectorLength, 0.5);
                        make_complex_tone_2(spaceDetectorTone, detectorLength, 
                            (float)lockedBinSpace * binScale, detectorLength, 0.5);
                    }
                } else {
                    //cout << blockCount << " " << maxBin << endl;
                }
            }

            if (locked) {
                float corrMark = complex_corr_2(&(fftBlock[fftN - detectorLength]), 
                    markDetectorTone, detectorLength);
                float corrSpace = complex_corr_2(&(fftBlock[fftN - detectorLength]), 
                    spaceDetectorTone, detectorLength);
                float diff = corrMark - corrSpace;
                cout << "mark " << corrMark << " space " << corrSpace << " diff " << diff << endl;
            }

            if (blockCount > 125) {
                return 0;
            }


            dataPtr += blockSize;
            blockCount++;
        }

        cout << endl;
    }
}
