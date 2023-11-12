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
#ifndef _Demodulator_h
#define _Demodulator_h

#include <cstdint>

#include "fixed_math.h"
#include "fixed_fft.h"
#include "DemodulatorListener.h"
#include "ClockRecoveryPLL.h"

namespace scamp {

class Demodulator {
public:

    Demodulator(DemodulatorListener* listener, uint16_t sampleFreq, uint16_t fftN,
        q15* fftTrigTableSpace, q15* fftWindowSpace, q15* bufferSpace);

    /**
     * Call this function at the rate defined by sampleFreq and pass the latest
     * sample from the ADC.  Everything happens here!
     */
    void tick(int16_t sample);

private: 

    DemodulatorListener* _listener;
    const uint16_t _sampleFreq;
    const uint16_t _fftN;
    FixedFFT _fft;
    q15* _fftWindow;
    ClockRecoveryPLL _pll;
    const uint16_t _blockSize = 32;

    const unsigned int _symbolCount = 2;

    // This is the approximate symbol rate for SCAMP FSK.  This is used
    // only to estimate the length of the "long mark"    
    const unsigned int _samplesPerSymbol = 60;
    // Calculate the duration of the block in seconds
    const float _blockDuration = (float)_blockSize / (float)_sampleFreq;
    // Calculate the symbol duration in seconds
    const float _symbolDuration = (float)_samplesPerSymbol / (float)_sampleFreq;
    // Calculate the duration of the "long mark"
    const float _longMarkDuration = 24.0 * _symbolDuration;
    // Calculate the number of blocks that make up the long mark and
    // round down.  We give this a slight haircut to improve robustness.
    const uint16_t _longMarkBlocks = ((_longMarkDuration / _blockDuration) * 0.70);

    uint32_t _bufferPtr = 0;
    q15* _buffer; 
};

}

#endif

