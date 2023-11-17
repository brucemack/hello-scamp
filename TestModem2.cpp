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
#include <cmath>
#include <iostream>
#include "TestModem2.h"

using namespace std;

namespace scamp {

TestModem2::TestModem2(float* samples, unsigned int samplesSize, 
    unsigned int sampleRate,
    unsigned int samplesPerSymbol,
    unsigned int markFreq, unsigned int spaceFreq) 
:   _samples(samples),
    _samplesSize(samplesSize),
    _sampleRate(sampleRate),
    _samplesPerSymbol(samplesPerSymbol),
    _markFreq(markFreq),
    _spaceFreq(spaceFreq) {
}

void TestModem2::sendSilence() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = 0;
        }
    }
}

void TestModem2::sendHalfSilence() {
    for (unsigned int i = 0; i < _samplesPerSymbol / 2; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = 0;
        }
    }
}

void TestModem2::sendMark() {
    float omega = 2.0f * 3.1415926f * (float)_markFreq / (float)_sampleRate;
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = std::cos(_phi);
            _phi += omega;
        }
    }
}

void TestModem2::sendSpace() {
    float omega = 2.0f * 3.1415926f * (float)_spaceFreq / (float)_sampleRate;
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = std::cos(_phi);
            _phi += omega;
        }
    }
}

}

