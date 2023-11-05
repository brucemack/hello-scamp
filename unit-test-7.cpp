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

static void shift_block(q15* fftBlock, uint16_t fftN, uint16_t blockSize) {
    for (uint16_t i = 0; i < fftN; i++) {
        if (i < fftN - blockSize) {
            fftBlock[i] = fftBlock[i + blockSize];
        } else {
            fftBlock[i] = 0;
        }
    }
}

int main(int argc, const char** argv) {

    const unsigned int sampleFreq = 2000;
    const unsigned int markFreq = 667;
    const unsigned int spaceFreq = 600;
    const unsigned int samplesPerSymbol = 60;

    const unsigned int S = 32 * 30 * samplesPerSymbol;
    float samples[S];
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
            std::ofstream outfile("bin/scamp-fsk-demo-1.txt");
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
        for (unsigned int i = 0; i < 15; i++)
            modem2.sendSilence();
        // Transmit a legit message
        for (unsigned int i = 0; i < frameCount; i++) {
            frames[i].transmit(modem2);
        }
        for (unsigned int i = 0; i < 15; i++)
            modem2.sendSilence();
    }

    // Now decode
    //ostringstream outStream;
    {
        ClockRecoveryPLL pll(sampleFreq);
        // Purposely set for the wrong frequency to watch the clock 
        // recovery work. But it's close.
        pll.setBitFrequencyHint(36);

        // Start a sampling loop (one block at a time)
        const uint16_t blockSize = 32;
        // This is the pointer into the main datastream
        uint16_t dataPtr = 0;

        const uint16_t fftN = 512;

        q15 fftBlock[fftN];
        q15 window[fftN];
        FixedFFT<fftN> fft;

        for (uint16_t i = 0; i < fftN; i++) {
            // Build the window (raised cosine)
            window[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * PI * ((float) i) / ((float)fftN))));
            fftBlock[i] = 0;
        }

        // Walk through the data one block at a time
        uint16_t blockCount = 0;

        while (dataPtr + blockSize < modem2.getSamplesUsed()) {            

            cout << "----- " << blockCount++ << " -------" << endl;

            // Shift the FFT block 
            shift_block(fftBlock, fftN, blockSize);

            // Fill in the top part of the FFT block with the new data
            for (uint16_t i = 0; i < blockSize; i++) {
                fftBlock[fftN - blockSize + i] = f32_to_q15(samples[dataPtr + i]);
            }

            // Do the FFT in a separate buffer
            cq15 x[fftN];
            for (uint16_t i = 0; i < fftN; i++) {
                x[i].r = fftBlock[i];
                x[i].i = 0;
            }
            fft.transform(x);

            // Find the largest power
            unsigned int b = max_idx(x, 1, (fftN / 2) - 1);
            cout << "Max idx " << b << endl;
            cout << "Max mag " << x[b].mag_f32() << endl;
            cout << "Max frq " << fft.binToFreq(b, sampleFreq) << endl;

            dataPtr += blockSize;
        }
    }
}
