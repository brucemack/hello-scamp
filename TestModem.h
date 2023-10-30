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
#ifndef _TestModem_h
#define _TestModem_h

#include <cstdint>
#include "Modulator.h"

namespace scamp {

class TestModem : public Modulator {
public:

    TestModem(int8_t* samples, unsigned int tonesSize, unsigned int samplesPerSymbol);

    virtual void sendSilence();
    virtual void sendHalfSilence();
    virtual void sendMark();
    virtual void sendSpace();

    unsigned int getSamplesUsed() const { return _samplesUsed; }

private:

    int8_t* _samples;
    unsigned int _samplesSize;
    unsigned int _samplesUsed;
    unsigned int _samplesPerSymbol;
};

}

#endif
