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

    virtual void dataSyncAcquired();
    virtual void frequencyLocked(uint16_t markFreq, uint16_t spaceFreq);
    virtual void badFrameReceived(uint32_t rawFrame);
    virtual void goodFrameReceived();

    virtual void received(char asciiChar);

    std::string getMessage() const;

private:

    std::ostringstream _out;
};

}

#endif
