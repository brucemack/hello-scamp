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
#include <cstdint>

#include "Util.h"
#include "Demodulator.h"

namespace scamp {

Demodulator::Demodulator(DemodulatorListener* listener, 
    uint16_t sampleFreq, uint16_t fftN, q15* fftTrigTable, q15* fftWindow,
    cq15* fftResultSpace, q15* bufferSpace)
:   _listener(listener),
    _sampleFreq(sampleFreq),
    _fftN(fftN),
    _fftWindow(fftWindow),
    _fftResult(fftResultSpace),
    _fft(fftN, fftTrigTable),
    _pll(sampleFreq),
    _buffer(bufferSpace) { 

    // NOTICE: We are purposely setting the initial frequency slightly 
    // wrong to show that PLL will adjust accordingly.
    _pll.setBitFrequencyHint(36);

    // Build the Hann window for the FFT (raised cosine)
    for (uint16_t i = 0; i < _fftN; i++) {
        _fftWindow[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)_fftN))));
    }

    memset((void*)_buffer, 0, _fftN);
    memset((void*)_maxBinHistory, 0, sizeof(_maxBinHistory));
    memset((void*)_demodulatorTone, 0, sizeof(_demodulatorTone));
    memset((void*)_symbolCorrFilter, 0, sizeof(_symbolCorrFilter));
}

void Demodulator::processSample(q15 sample) {

    // Capture the sample in the circular buffer            
    _buffer[_bufferPtr] = sample;
    // Remember where the reading starts
    const uint16_t readBufferPtr = _bufferPtr;
    // Increment the write pointer and wrap if needed
    _bufferPtr = (_bufferPtr + 1) % _fftN;

    // Did we just finish a new block?  If so, run the FFT
    if (_bufferPtr % _blockSize == 0) {
        
        _blockCount++;

        // Do the FFT in a the result buffer, including the window.  
        for (uint16_t i = 0; i < _fftN; i++) {
            _fftResult[i].r = mult_q15(_buffer[wrapIndex(readBufferPtr, i, _fftN)], 
                _fftWindow[i]);
            _fftResult[i].i = 0;
        }

        _fft.transform(_fftResult);

        // Find the largest power. Notice that we ignore bin 0 (DC)
        // since that's not relevant.
        const uint16_t maxBin = max_idx(_fftResult, 1, _fftN / 2);

        // If we are not yet frequency locked, try to lock
        if (!_frequencyLocked) {

            // Find the total power
            float totalPower = 0;
            for (uint16_t i = 0; i < _fftN / 2; i++) {
                totalPower += _fftResult[i].mag_f32_squared();
            }
            // Find the percentage of power at the max (and two adjacent)
            float maxBinPower = _fftResult[maxBin].mag_f32_squared();
            if (maxBin > 1) {
                maxBinPower += _fftResult[maxBin - 1].mag_f32_squared();
            }
            if (maxBin < (_fftN / 2) - 1) {
                maxBinPower += _fftResult[maxBin + 1].mag_f32_squared();
            }
            const float maxBinPowerFract = maxBinPower / totalPower;

            // Shift the history collection area and accumulate the new 
            // observation.
            for (uint16_t i = 0; i < _maxBinHistorySize - 1; i++) {
                _maxBinHistory[i] = _maxBinHistory[i + 1];
            }
            _maxBinHistory[_maxBinHistorySize - 1] = maxBin;

            // Find the variance of the max bin across the recent observations
            // to see if we have stability.  We are only looking 
            float maxBinMean = 0;
            uint16_t binHistoryStart;
            uint16_t binHistoryLength;
            if (_longMarkBlocks > _maxBinHistorySize) {
                binHistoryStart = 0;
                binHistoryLength = _maxBinHistorySize;
            } else {
                binHistoryStart = _maxBinHistorySize - _longMarkBlocks;
                binHistoryLength = _longMarkBlocks;
            }
            for (uint16_t i = 0; i < binHistoryLength; i++) {
                maxBinMean += _maxBinHistory[binHistoryStart + i];
            }
            maxBinMean /= (float)binHistoryLength;

            // The locking logic only works when the history is full
            if (_blockCount >= binHistoryLength) {

                float maxBinVar = 0;
                for (uint16_t i = 0; i < binHistoryLength; i++) {
                    if (_maxBinHistory[i] != 0) {
                        maxBinVar += std::pow(_maxBinHistory[i] - maxBinMean, 2);
                    }
                }
                maxBinVar /= ((float)binHistoryLength - 1);
                float maxBinStd = std::sqrt(maxBinVar);

                // If the standard deviation is small and the power is large then 
                // assume we're seeing the "long mark"                
                if (maxBinStd <= 1.0 && maxBinPowerFract > 0.10) {
                    _frequencyLocked = true;
                    _lockedBinMark = maxBin;
                    float lockedBinSpread = ((float)_symbolSpreadHz / (float)_sampleFreq) * 
                        (float)_fftN;
                    float lockedBinSpace = ((float)_lockedBinMark - lockedBinSpread);

                    float lockedMarkHz = (float)_lockedBinMark * (float)_sampleFreq / (float)_fftN;
                    float lockedSpaceHz = lockedBinSpace * (float)_sampleFreq / (float)_fftN;

                    make_complex_tone_2(_demodulatorTone[0], _demodulatorToneN, 
                        (float)lockedBinSpace, _fftN, 0.5);
                    make_complex_tone_2(_demodulatorTone[1], _demodulatorToneN, 
                        (float)_lockedBinMark, _fftN, 0.5);

                    _listener->frequencyLocked(lockedMarkHz, lockedSpaceHz);
                }
            }
        }
    }

    // ----- Quadrature Demodulation -----------------------------------------

    // Figure out the detection starting point. Back up the length of the 
    // demodulator series, wrapping as necessary.
    // 
    // TEST CASE 2: fftN = 512, demodToneN = 16, readBufferPtr = 15
    // We need to start at 511.  Gap = 16-15 = 1, start = 512 - 1
    //
    uint16_t demodulatorStart = 0;
    if (readBufferPtr >= _demodulatorToneN) {
        demodulatorStart = readBufferPtr - _demodulatorToneN;
    } else {
        uint16_t gap = _demodulatorToneN - readBufferPtr;
        demodulatorStart = _fftN - gap;
    }
    // Correlate recent history with each of the symbol tones
    for (uint16_t s = 0; s < _symbolCount; s++) {
        // HERE WE HAVE AUTOMATIC WRAPPING
        _symbolCorr[s] = complex_corr_2(_buffer, demodulatorStart, _fftN, 
            _demodulatorTone[s], _demodulatorToneN);
        // Here we keep track of some recent history of the symbol 
        // correlation.
        _symbolCorrFilter[s][_symbolCorrFilterPtr] = _symbolCorr[s];
    }
    _symbolCorrFilterPtr = (_symbolCorrFilterPtr + 1) % _symbolCorrFilterN;

    // Calculate the recent max and average correlation of each symbol
    // from the history series.
    for (uint16_t s = 0; s < _symbolCount; s++) {
        _symbolCorrAvg[s] = 0;
        _symbolCorrMax[s] = 0;
        for (uint16_t n = 0; n < _symbolCorrFilterN; n++) {
            float corr = _symbolCorrFilter[s][n];
            _symbolCorrAvg[s] += corr;
            _symbolCorrMax[s] = std::max(_symbolCorrMax[s], corr);
        }
        _symbolCorrAvg[s] /= (float)_symbolCorrFilterN;
    }

    // Look for an inflection point in the respective correlations 
    // of the symbols.  
    if (_activeSymbol == 0) {
        if (_symbolCorrAvg[1] > _symbolCorrAvg[0]) {
            _activeSymbol = 1;
            _listener->bitTransitionDetected();
        }
    } else {
        if (_symbolCorrAvg[0] > _symbolCorrAvg[1]) {
            _activeSymbol = 0;
            _listener->bitTransitionDetected();
        }
    }

    // Show the sample to the PLL for clock recovery
    bool capture = _pll.processSample(_activeSymbol == 1);
        
    // Process the sample if we are told to do so by the data clock
    // recovery PLL.
    if (capture) {

        // Bring in the next bit. 
        _frameBitAccumulator <<= 1;
        _frameBitAccumulator |= (_activeSymbol == 1) ? 1 : 0;
        _listener->receivedBit(_activeSymbol == 1, _frameBitCount);
        _frameBitCount++;
        
        if (!_inDataSync) {
            // Look for sync frame, or something very close to it.
            if (abs(Frame30::correlate30(_frameBitAccumulator, Frame30::SYNC_FRAME.getRaw())) > 29) {
                _inDataSync = true;
                _frameBitCount = 0;
                _listener->dataSyncAcquired()                        ;
            }
        }
        // Here we are consuming real data frames
        else {
            if (_frameBitCount == 30) {
                _frameBitCount = 0;
                _frameCount++;
                Frame30 frame(_frameBitAccumulator & Frame30::MASK30LSB);
                if (!frame.isValid()) {      
                    _listener->badFrameReceived(frame.getRaw());
                } else {
                    _listener->goodFrameReceived();
                    CodeWord24 cw24 = frame.toCodeWord24();
                    CodeWord12 cw12 = cw24.toCodeWord12();
                    Symbol6 sym0 = cw12.getSymbol0();
                    Symbol6 sym1 = cw12.getSymbol1();
                    if (sym0.getRaw() != 0) {
                        _listener->received(sym0.toAscii());
                    }
                    if (sym1.getRaw() != 0) {
                        _listener->received(sym1.toAscii());
                    }
                }
            }
        }
    }
}

}

