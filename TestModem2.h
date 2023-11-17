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
#ifndef _TestModem2_h
#define _TestModem2_h

#include <cstdint>
#include "Modulator.h"

namespace scamp {

/**
 * Modulates into a memory buffer.
 */
class TestModem2 : public Modulator {
public:

    TestModem2(float* samples, unsigned int samplesSize, unsigned int sampleRate,
        unsigned int samplesPerSymbol,
        unsigned int markFreq, unsigned int spaceFreq,
        float amp, float dcBias);
    virtual ~TestModem2() { }

    virtual void sendSilence();
    virtual void sendHalfSilence();
    virtual void sendMark();
    virtual void sendSpace();

    uint32_t getSamplesUsed() const { return _samplesUsed; }

private:

    float* _samples;
    unsigned int _samplesSize;
    uint32_t _samplesUsed = 0;
    unsigned int _sampleRate;
    unsigned int _samplesPerSymbol;
    unsigned int _markFreq;
    unsigned int _spaceFreq;
    float _amp;
    float _dcBias;
    float _phi = 0;
};

}

#endif
