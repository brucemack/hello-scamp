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
#include <fstream>
#include <cmath>
#include <sstream>

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
#include "TestDemodulatorListener.h"

using namespace std;
using namespace scamp;

// ------ Data Area -------

const unsigned int sampleFreq = 2000;
const unsigned int samplesPerSymbol = 60;
const unsigned int symbolCount = 2;
const unsigned int markFreq = 667;
const unsigned int spaceFreq = 600;
// Here we can inject a tuning error to show that the demodulator will
// still find the signal.
const unsigned int tuningErrorHz = 100;
const unsigned int S = 34 * 30 * samplesPerSymbol;
static float samples[S];

// The size of the FFT used for frequency acquisition
const uint16_t fftN = 512;
static q15 buffer[fftN];

int main(int argc, const char** argv) {

    cout << "SCAMP Modem Demonstration 7a" << endl;
    cout << "  Sample Freq    : " << sampleFreq << endl;
    cout << "  Mark           : " << markFreq + tuningErrorHz << endl;
    cout << "  Space          : " << spaceFreq + tuningErrorHz << endl;
    cout << "  Samples/Symbol : " << samplesPerSymbol << endl;

    // This is the modem used for the demonstration.  Samples
    // are written to a memory buffer.
    TestModem2 modem2(samples, S, sampleFreq, samplesPerSymbol, 
        markFreq + tuningErrorHz, spaceFreq + tuningErrorHz);

    // This is a modem that is used to capture the data for printing.
    int8_t printSamples[34 * 30];
    TestModem modem3(printSamples, sizeof(printSamples), 1);
    
    // =========================================================================
    // Make a message, encode it, and then recover it. In this test we are 
    // using the SCAMP FSK (33.3 bits per second) format.  
    {        
        const char* testMessage = "DE KC1FSZ, GOOD MORNING";

        // Encode the message.  This leads to about 25K samples.
        Frame30 frames[32];
        unsigned int frameCount = encodeString(testMessage, frames, 32, true);
    
        // We purposely offset the data stream by a half symbol 
        // to stress the PLL.
        //modem.sendHalfSilence();
        // This silence is 30 symbols, or 30 * 60 = 180 samples long
        for (unsigned int i = 0; i < 30; i++)
            modem2.sendSilence();
        // Transmit a legit message
        for (unsigned int i = 0; i < frameCount; i++) {
            frames[i].transmit(modem2);
            frames[i].transmit(modem3);
        }
        for (unsigned int i = 0; i < 30; i++) {
            modem2.sendSilence();
        }
    }

    // Display
    {
        cout << endl << "Sending these frames:" << endl;
        for (uint16_t i = 0; i < modem3.getSamplesUsed(); i++) {
            if (i % 30 == 0) {
                cout << endl;
            }
            if (printSamples[i] == 1) {
                cout << "1";
            } else if (printSamples[i] == -1) {
                cout << "0";
            } else {
                cout << "?";
            }
        }
        cout << endl << endl;
    }

    // Now decode without any prior knowledge of the frequency or phase
    // of the transmitter.
    {
        TestDemodulatorListener testListener;
        DemodulatorListener* listener = &testListener;

        ClockRecoveryPLL pll(sampleFreq);
        // NOTICE: We are purposely setting the initial frequency slightly 
        // wrong to show that PLL will adjust accordingly.
        pll.setBitFrequencyHint(36);
        // The samples are processed one block at a time. This is the
        // size of the block.
        const uint16_t blockSize = 32;
        // Calculate the duration of the block in seconds
        const float blockDuration = (float)blockSize / (float)sampleFreq;
        // Calculate the symbol duration in seconds
        const float symbolDuration = (float)samplesPerSymbol / (float)sampleFreq;
        // Calculate the duration of the "long mark"
        const float longMarkDuration = 24.0 * symbolDuration;
        // Calculate the number of blocks that make up the long mark and
        // round down.  We give this a slight haircut to improve robustness.
        const uint16_t longMarkBlocks = ((longMarkDuration / blockDuration) * 0.70);
       
        // This is the pointer into the main data stream
        uint32_t samplePtr = 0;
        uint32_t bufferPtr = 0;

        q15 window[fftN];
        q15 trigTable[fftN];

        const FixedFFT fft(fftN, trigTable);

        // Build the Hann window for the FFT (raised cosine)
        for (uint16_t i = 0; i < fftN; i++) {
            window[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)fftN))));
        }

        // This is where we store the history of the bin with the largest 
        // magnitude.
        const uint16_t blockHistorySize = longMarkBlocks;
        uint16_t maxBinHistory[blockHistorySize];
        for (uint16_t i = 0; i < blockHistorySize; i++) {
            maxBinHistory[i] = 0;
        }

        bool frequencyLocked = false;
        uint16_t lockedBinMark = 0;
        uint16_t blockCount = 0;
        uint16_t blockPtr = 0;
        uint16_t activeSymbol = 0;

        bool inDataSync = false;
        // Here is where we accumulate data bits
        uint32_t frameBitAccumulator = 0;
        // The number of bits received in the frame
        uint16_t frameBitCount = 0;
        // The number of frames received
        uint16_t frameCount = 0;

        // These buffers are loaded based on the frequency that the decoder
        // decides to lock onto.
        // TODO: DO WE NEED A WINDOW HERE?
        const uint16_t demodulatorToneN = 16;
        cq15 demodulatorTone[symbolCount][demodulatorToneN];

        // These buffers hold the history of the detection of the symbol tones
        const uint16_t symbolCorrFilterN = 4;
        uint16_t symbolCorrFilterPtr = 0;
        float symbolCorrFilter[symbolCount][symbolCorrFilterN];

        // Walk through the data one byte at a time.  We do something extra
        // each time we have processed a complete block.
        while (samplePtr < modem2.getSamplesUsed()) {            

            const q15 sample = f32_to_q15(samples[samplePtr++]);
            // Capture the sample in the circular buffer            
            buffer[bufferPtr] = sample;
            // Remember where the reading starts
            const uint16_t readBufferPtr = bufferPtr;
            // Increment the write pointer and wrap if needed
            bufferPtr = (bufferPtr + 1) % fftN;

            // Did we just finish a new block?  If so, run the FFT
            if (bufferPtr % blockSize == 0) {
                
                blockCount++;

                // Do the FFT in a separate buffer, including the window.  
                cq15 x[fftN];
                for (uint16_t i = 0; i < fftN; i++) {
                    x[i].r = mult_q15(buffer[wrapIndex(readBufferPtr, i, fftN)], 
                        window[i]);
                    x[i].i = 0;
                }
                fft.transform(x);

                // Find the largest power. Notice that we ignore bin 0 (DC)
                // since that's not relevant.
                const uint16_t maxBin = max_idx(x, 1, fftN / 2);
                const unsigned int maxFreq = fft.binToFreq(maxBin, sampleFreq);

                // If we are not yet frequency locked, try to lock
                if (!frequencyLocked) {

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

                        // If the standard deviation is small and the power is large then 
                        // assume we're seeing the "long mark"                
                        if (maxBinStd <= 1.0 && maxBinPowerFract > 0.10) {
                            frequencyLocked = true;
                            lockedBinMark = maxBin;
                            float lockedBinSpread = ((float)(markFreq - spaceFreq) / (float)sampleFreq) * 
                                (float)fftN;
                            float lockedBinSpace = ((float)lockedBinMark - lockedBinSpread);

                            float lockedMarkHz = (float)lockedBinMark * (float)sampleFreq / (float)fftN;
                            float lockedSpaceHz = lockedBinSpace * (float)sampleFreq / (float)fftN;

                            make_complex_tone_2(demodulatorTone[0], demodulatorToneN, 
                                (float)lockedBinSpace, fftN, 0.5);
                            make_complex_tone_2(demodulatorTone[1], demodulatorToneN, 
                                (float)lockedBinMark, fftN, 0.5);

                            listener->frequencyLocked(lockedMarkHz, lockedSpaceHz);
                        }
                    }
                }
            }

            // ----- Quadrature Demodulation -----------------------------------------

            // Figure out the detection starting point. Back up the length of the 
            // demodulator series, wrapping as necessary.
            uint16_t demodulatorStart = (((int16_t)fftN + (int16_t)readBufferPtr - 
                (int16_t)demodulatorToneN)) % fftN;
            // Correlate recent history with each of the symbol tones
            float symbolCorr[symbolCount];
            for (uint16_t s = 0; s < symbolCount; s++) {
                symbolCorr[s] = complex_corr_2(buffer, demodulatorStart, fftN, 
                    demodulatorTone[s], demodulatorToneN);
                // Here we keep track of some recent history of the symbol 
                // correlation.
                symbolCorrFilter[s][symbolCorrFilterPtr] = symbolCorr[s];
            }
            symbolCorrFilterPtr = (symbolCorrFilterPtr + 1) % symbolCorrFilterN;

            // Calculate the recent max and average correlation of each symbol
            // from the history series.
            float symbolCorrAvg[symbolCount];
            float symbolCorrMax[symbolCount];
            for (uint16_t s = 0; s < symbolCount; s++) {
                symbolCorrAvg[s] = 0;
                symbolCorrMax[s] = 0;
                for (uint16_t n = 0; n < symbolCorrFilterN; n++) {
                    float corr = symbolCorrFilter[s][n];
                    symbolCorrAvg[s] += corr;
                    symbolCorrMax[s] = std::max(symbolCorrMax[s], corr);
                }
                symbolCorrAvg[s] /= (float)symbolCorrFilterN;
            }

            // Look for an inflection point in the respective correlations 
            // of the symbols.  
            bool edgeDetected = false;
            if (activeSymbol == 0) {
                if (symbolCorrAvg[1] > symbolCorrAvg[0]) {
                    activeSymbol = 1;
                    edgeDetected = true;
                }
            } else {
                if (symbolCorrAvg[0] > symbolCorrAvg[1]) {
                    activeSymbol = 0;
                    edgeDetected = true;
                }
            }

            // Show the edge to the PLL 
            bool capture = pll.processSample(activeSymbol == 1);
                
            // Process the sample if we are told to do so by the data clock
            // recovery PLL.
            if (capture) {

                // Bring in the next bit. 
                frameBitAccumulator <<= 1;
                frameBitAccumulator |= (activeSymbol == 1) ? 1 : 0;
                frameBitCount++;
                
                if (!inDataSync) {
                    // Look for sync frame, or something very close to it.
                    if (abs(Frame30::correlate30(frameBitAccumulator, Frame30::SYNC_FRAME.getRaw())) > 29) {
                        inDataSync = true;
                        frameBitCount = 0;
                        listener->dataSyncAcquired()                        ;
                    }
                }
                // Here we are consuming real data frames
                else {
                    if (frameBitCount == 30) {
                        frameBitCount = 0;
                        frameCount++;
                        Frame30 frame(frameBitAccumulator & Frame30::MASK30LSB);
                        if (!frame.isValid()) {      
                            listener->badFrameReceived(frame.getRaw());
                        } else {
                            listener->goodFrameReceived();
                            CodeWord24 cw24 = frame.toCodeWord24();
                            CodeWord12 cw12 = cw24.toCodeWord12();
                            Symbol6 sym0 = cw12.getSymbol0();
                            Symbol6 sym1 = cw12.getSymbol1();
                            if (sym0.getRaw() != 0) {
                                listener->received(sym0.toAscii());
                            }
                            if (sym1.getRaw() != 0) {
                                listener->received(sym1.toAscii());
                            }
                        }
                    }
                }
            }
        }
        cout << "MESSAGE: " << testListener.getMessage() << endl;
    }
}

// FIRST DATA FRAME
//
// 01111 10000 01111 01000 01010 10001
// 01234 56789 01234 56789 01234 56789

// DATA SYNC IN 89
