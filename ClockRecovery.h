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
#ifndef _ClockRecovery_H
#define _ClockRecovery_H

#include <cstdint>

namespace scamp {

class ClockRecovery {
public:

    /**
     * This is the main function.  Call this at the sample rate frequency with 
     * an observation of the incoming signal.  If the return is true then the 
     * caller should use the sample as the observation of the curent bit. If
     * the return is false then just ignore the sample and keep going.
     */
    virtual bool processSample(uint8_t symbol) = 0;

    /**
     * @returns The lock frequency in Hertz.
     */
    virtual uint32_t getClockFrequency() const = 0;

    /**
     * @returns The number of samples received since the last edge
     *   transition.
     */
    virtual uint16_t getSamplesSinceEdge() const = 0;

    /**
     * @returns The last phase error from -1 to +1. 0 Means the phase is perfect.
     */
    virtual float getLastPhaseError() const = 0;
};

}

#endif