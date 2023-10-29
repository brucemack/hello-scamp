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
#ifndef _ClockRecoveryPLL_H
#define _ClockRecoveryPLL_H

#include <cstdint>

namespace scamp {

    /**
     * A PLL class used for recovering the bit clock on an input stream.
    */
    class ClockRecoveryPLL {
    public:

        ClockRecoveryPLL(unsigned int sampleRate);

        /**
         * Tells the PLL the approximate bit frequency that it is
         * looking for. This improves the speed of the lock.
         */
        void setBitFrequencyHint(unsigned int hz);

        /**
         * This is the main function.  Call this at the sample rate frequency with 
         * an observation of the incoming signal.  If the return is true then the 
         * caller should use the sample as an observation of the curent bit.
        */
        bool processSample(bool sample);

        int32_t getIntegration() const { return _integration; }
        int32_t getLastError() const { return _lastError; }
        uint32_t getFrequency() const;

    private:

        void _edgeDetected();
        
        bool _idle;
        uint16_t _sampleRate;
        int32_t _integration;
        int16_t _omega;
        uint16_t _phi;
        uint16_t _targetPhi;
        int16_t _Kp;
        int16_t _Ki;
        int16_t _bias;
        int32_t _lastError;
        uint16_t _lastPhi;
        bool _lastSample;
    };
};

#endif