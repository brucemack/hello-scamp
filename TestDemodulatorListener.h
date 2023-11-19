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
#ifndef _TestDemodulatorListener_h
#define _TestDemodulatorListener_h

#include <cstdint>
#include <sstream>

#include "DemodulatorListener.h"

namespace scamp {

/**
 * An abstract interface used to receive status/events from the demodulator
 */
class TestDemodulatorListener : public DemodulatorListener {
public:

    /**
     * NOTE: Each of these takes approximately 20 bytes, so one second consumes
     * about 40K.  
    */
    struct Sample {
        uint8_t activeSymbol;
        uint8_t capture;
        int32_t pllError;
        float symbolCorr[2];
        float symbolCorrAvg[2];
        float maxCorr;
    };

    enum TriggerMode { NONE, MANUAL, ON_LOCK };

    TestDemodulatorListener(std::ostream& out);
    TestDemodulatorListener(std::ostream& out,
        Sample* sampleSpace, uint16_t sampleSpaceSize);

    virtual void dataSyncAcquired();
    virtual void frequencyLocked(uint16_t markFreq, uint16_t spaceFreq);
    virtual void badFrameReceived(uint32_t rawFrame);
    virtual void goodFrameReceived();
    virtual void received(char asciiChar);
    virtual void sampleMetrics(uint8_t activeSymbol, bool capture, 
        int32_t pllError,
        float* symbolCorr, float* symbolCorrAvg, float maxCorr);
    virtual void receivedBit(bool bit, uint16_t frameBitPos, int syncFrameCorr);

    std::string getMessage() const;

    void setTriggerMode(TriggerMode mode);
    void setTriggered(bool triggered);
    void setTriggerDelay(uint16_t delaySamples);

    void clearSamples();
    void dumpSamples(std::ostream& str) const;

private:

    std::ostream& _out;
    std::ostringstream _msg;
    Sample* _sampleSpace = 0;
    uint16_t _sampleSpaceSize = 0;
    uint16_t _sampleSpacePtr = 0;
    bool _triggered = false;
    TriggerMode _triggerMode = NONE;
    uint16_t _triggerDelay = 0;
    uint16_t _delayCounter = 0;
};

}

#endif
