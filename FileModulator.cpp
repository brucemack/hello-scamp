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
#include "FileModulator.h"

namespace scamp {

FileModulator::FileModulator(std::ostream& str, unsigned int sampleRate, unsigned int symbolLength, 
    unsigned int markFreq, unsigned int spaceFreq) 
:   _str(str), 
    _sampleRate(sampleRate),
    _symbolLength(symbolLength),
    _markFreq(markFreq),
    _spaceFreq(spaceFreq),
    _phi(0)
{
}

void FileModulator::sendMark() {
    _send(_markFreq);
}

void FileModulator::sendSpace() {
    _send(_spaceFreq);
}

void FileModulator::sendSilence() { 
    for (unsigned int i = 0; i < _symbolLength; i++) {
        _str << (int)0 << "\n";
    }
    _str.flush();
}

void FileModulator::_send(unsigned int freq) {

    const float scale = 32760;
    const float omega = 2.0 * 3.14159 * (float)freq / (float)_sampleRate;

    for (unsigned int i = 0; i < _symbolLength; i++) {
        float s = std::cos(_phi) * scale;
        _str << (int)s << "\n";
        _phi += omega;
    }

    _str.flush();
}

}