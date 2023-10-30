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
#include "TestModem.h"

namespace scamp {

TestModem::TestModem(int8_t* samples, unsigned int samplesSize, unsigned int samplesPerSymbol) 
:   _samples(samples),
    _samplesSize(samplesSize),
    _samplesUsed(0),
    _samplesPerSymbol(samplesPerSymbol) {
}

void TestModem::sendSilence() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) 
        _samples[_samplesUsed++] = 0;
}

void TestModem::sendMark() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) 
        _samples[_samplesUsed++] = 1;
}

void TestModem::sendSpace() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) 
        _samples[_samplesUsed++] = -1;
}

}

