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
#ifndef _DemodulatorListener_h
#define _DemodulatorListener_h

#include <cstdint>

namespace scamp {

/**
 * An abstract interface used to receive status/events from the demodulator
 */
class DemodulatorListener {
public:

    virtual void dataSyncAcquired() { }
    virtual void dataSyncLost() { }
    virtual void frequencyLocked(uint16_t markFreq, uint16_t spaceFreq) { }
    virtual void goodFrameReceived() { }
    virtual void badFrameReceived(uint32_t rawFrame) { }
    virtual void received(char asciiChar) { }
    virtual void bitTransitionDetected() { }
    virtual void receivedBit(bool bit, uint16_t frameBitPos, int syncFrameCorr) { }
    // The key data used to identify symbols
    virtual void sampleMetrics(uint8_t activeSymbol, bool capture, 
        int32_t lastPLLError,
        float* symbolCorr, float* symbolCorrAvg, float maxCorr) { }
    /**
     * Called when the receiver discards a duplicate codeword
     */
    virtual void discardedDuplicate() { }
};

}

#endif

