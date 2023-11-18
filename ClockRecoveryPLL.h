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

        /**
         * @param sampleRate Should be set to the base sampling frequency
         *   in Hertz.  For SCAMP this would typically be 2000.
        */
        ClockRecoveryPLL(unsigned int sampleRate = 2000);

        /**
         * Tells the PLL the approximate bit frequency that it is
         * looking for. This improves the speed of the lock.
         * 
         * @param hz The approximate (starting) lock frequency in Hertz.
         */
        void setBitFrequencyHint(unsigned int hz);

        /**
         * This is the main function.  Call this at the sample rate frequency with 
         * an observation of the incoming signal.  If the return is true then the 
         * caller should use the sample as the observation of the curent bit. If
         * the return is false then just ignore the sample and keep going.
         */
        bool processSample(bool sample);

        int32_t getIntegration() const { return _integration; }
        int32_t getLastError() const { return _lastError; }

        /**
         * @returns The lock frequency in Hertz.
         */
        uint32_t getFrequency() const;

        /**
         * @returns The number of samples received since the last edge
         *   transition.
         */
        uint16_t getSamplesSinceEdge() const { return _samplesSinceEdge; }

    private:

        void _edgeDetected();
        
        bool _idle;
        const uint16_t _sampleRate;
        int32_t _integration = 0;
        int16_t _omega = 0;
        uint16_t _phi = 0;
        uint16_t _targetPhi;
        const int16_t _Kp = 7;
        const int16_t _Ki = 10;
        int16_t _offset;
        int32_t _lastError = 0;
        uint16_t _lastPhi = 0;
        bool _lastSample = false;
        uint16_t _samplesSinceEdge = 0;
    };
}

#endif