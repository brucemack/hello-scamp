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
#include "ClockRecoveryDLL.h"

#define SYMBOL_COUNT (2)

namespace scamp {

/**
 * This contains all of the SCAMP demodulator logic.  This is a stateful
 * object that will be called once every sample interval with the latest
 * available sample.
 */
class Demodulator {
public:

    Demodulator(uint16_t sampleFreq, uint16_t lowestFreq,
        uint16_t log2fftN,
        q15* fftTrigTableSpace, q15* fftWindowSpace, cq15* fftResultSpace, 
        q15* bufferSpace);

    void setListener(DemodulatorListener* listener) { _listener = listener; };

    /**
     * Call this depending on the mode being used.
     */
    void setSymbolSpread(uint16_t spreadHz) { _symbolSpreadHz = spreadHz; };

    /**
     * Call this function at the rate defined by sampleFreq and pass the latest
     * sample from the ADC.  Everything happens here!
     */
    void processSample(q15 sample);

    /**
     * Call this function to clear the frequency lock and the data 
     * synchronization.
    */
    void reset();

    void setFrequencyLock(bool lock);

    bool isFrequencyLocked() const { return _frequencyLocked; }
    uint16_t getFrameCount() const { return _frameCount; };
    int32_t getPLLIntegration() const;
    float getClockRecoveryPhaseError() const;
    float getLastDCPower() const { return _lastDCPower; };
    uint16_t getMarkFreq() const;

private: 

    DemodulatorListener* _listener;
    const uint16_t _sampleFreq;
    const uint16_t _fftN;
    const uint16_t _log2fftN;
    // This is the first bin that we pay attention to
    uint16_t _firstBin;
    // Optional space passed in by user
    q15* _fftWindow;
    cq15* _fftResult;
    FixedFFT _fft;
    //ClockRecoveryPLL _dataClockRecovery;
    ClockRecoveryDLL _dataClockRecovery;

    // FFT is performed every time this number of samples is collected
    const uint16_t _blockSize = 32;
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

    // The distance between the two symbols in positive HZ. The
    // default here is relevant to the SCAMP FSK mode.
    uint16_t _symbolSpreadHz = 67;

    uint32_t _sampleCount = 0;

    // Here's where we put the recent sample history in order to build
    // up enough to run the spectral analysis.
    uint16_t _bufferPtr = 0;
    q15* _buffer; 

    float _lastDCPower = 0;

    // This is where we store the recent history of the loudest bin 
    const uint16_t _maxBinHistorySize = 64;
    //const uint16_t _maxBinHistoryBins = 4;
    uint16_t _maxBinHistory[64];
    // The power threshold used for detecting a valid signal
    // Power of 0.002 was measured with Vpp = 1.5v
    float _binPowerThreshold = 5.0e-4;

    // Indicates whether the demodulator is locked onto a specific frequency
    // or whether it is in frequency acquisition mode.
    bool _frequencyLocked = false;

    // The bin that has been selected to represent "mark"
    uint16_t _lockedBinMark = 0;
    uint16_t _blockCount = 0;
    uint8_t _activeSymbol = 0;

    // Have we seen the synchronization frame?
    bool _inDataSync = false;
    // Here is where we accumulate data bits
    uint32_t _frameBitAccumulator = 0;
    // The number of bits received in the frame
    uint16_t _frameBitCount = 0;
    // The number of frames received
    uint16_t _frameCount = 0;
    // The last code word that was received (used for duplicate detection)
    uint16_t _lastCodeWord12 = 0;

    // These buffers are loaded based on the frequency that the decoder
    // decides to lock onto. The signal is convolved with these tones
    // for demodulation.
    const unsigned int _symbolCount = SYMBOL_COUNT;
    const uint16_t _demodulatorToneN = 16;
    cq15 _demodulatorTone[SYMBOL_COUNT][16];

    // The most recent correlation for each symbol
    float _symbolCorr[SYMBOL_COUNT];

    const uint16_t _maxCorrHistoryN = 32;
    uint16_t _maxCorrHistoryPtr = 0;
    // This buffers hold the recent history of the max correlation.  This
    // is used to determine the threshold.
    float _maxCorrHistory[32];

    uint16_t _edgeRiseSampleCounter = 0;
    float _lastCorrDiff = 0;
    uint16_t _edgeRiseSampleLimit = 2;
};

}

#endif

