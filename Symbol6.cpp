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
#include "Symbol6.h"

using namespace std;

namespace scamp {

// NOTE: This array was taken from the SCAMP specification.
static const uint8_t SCAMP6_TO_ASCII8[] = {
   0x00, '\b', '\r',  ' ',  '!', '\"', '\'',  '(',
    ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',
    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
    '9',  ':',  ';',  '=',  '?',  '@',  'A',  'B',
    'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',
    'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',
    'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
   '\\',  '^',  '`',  '~', 0xff, 0xff, 0xff, 0xff
};

// NOTE: This array was created by calling writeReverse() and pasting the result.
static const uint8_t ASCII8_TO_SCAMP6[] = {
0x0, // 0
0xff, // Not Found: 1
0xff, // Not Found: 2
0xff, // Not Found: 3
0xff, // Not Found: 4
0xff, // Not Found: 5
0xff, // Not Found: 6
0xff, // Not Found: 7
0x1, // 8
0xff, // Not Found: 9
0xff, // Not Found: 10
0xff, // Not Found: 11
0xff, // Not Found: 12
0x2, // d
0xff, // Not Found: 14
0xff, // Not Found: 15
0xff, // Not Found: 16
0xff, // Not Found: 17
0xff, // Not Found: 18
0xff, // Not Found: 19
0xff, // Not Found: 20
0xff, // Not Found: 21
0xff, // Not Found: 22
0xff, // Not Found: 23
0xff, // Not Found: 24
0xff, // Not Found: 25
0xff, // Not Found: 26
0xff, // Not Found: 27
0xff, // Not Found: 28
0xff, // Not Found: 29
0xff, // Not Found: 30
0xff, // Not Found: 31
0x3, // 20
0x4, // 21
0x5, // 22
0xff, // Not Found: 35
0xff, // Not Found: 36
0xff, // Not Found: 37
0xff, // Not Found: 38
0x6, // 27
0x7, // 28
0x8, // 29
0x9, // 2a
0xa, // 2b
0xb, // 2c
0xc, // 2d
0xd, // 2e
0xe, // 2f
0xf, // 30
0x10, // 31
0x11, // 32
0x12, // 33
0x13, // 34
0x14, // 35
0x15, // 36
0x16, // 37
0x17, // 38
0x18, // 39
0x19, // 3a
0x1a, // 3b
0xff, // Not Found: 60
0x1b, // 3d
0xff, // Not Found: 62
0x1c, // 3f
0x1d, // 40
0x1e, // 41
0x1f, // 42
0x20, // 43
0x21, // 44
0x22, // 45
0x23, // 46
0x24, // 47
0x25, // 48
0x26, // 49
0x27, // 4a
0x28, // 4b
0x29, // 4c
0x2a, // 4d
0x2b, // 4e
0x2c, // 4f
0x2d, // 50
0x2e, // 51
0x2f, // 52
0x30, // 53
0x31, // 54
0x32, // 55
0x33, // 56
0x34, // 57
0x35, // 58
0x36, // 59
0x37, // 5a
0xff, // Not Found: 91
0x38, // 5c
0xff, // Not Found: 93
0x39, // 5e
0xff, // Not Found: 95
0x3a, // 60
0xff, // Not Found: 97
0xff, // Not Found: 98
0xff, // Not Found: 99
0xff, // Not Found: 100
0xff, // Not Found: 101
0xff, // Not Found: 102
0xff, // Not Found: 103
0xff, // Not Found: 104
0xff, // Not Found: 105
0xff, // Not Found: 106
0xff, // Not Found: 107
0xff, // Not Found: 108
0xff, // Not Found: 109
0xff, // Not Found: 110
0xff, // Not Found: 111
0xff, // Not Found: 112
0xff, // Not Found: 113
0xff, // Not Found: 114
0xff, // Not Found: 115
0xff, // Not Found: 116
0xff, // Not Found: 117
0xff, // Not Found: 118
0xff, // Not Found: 119
0xff, // Not Found: 120
0xff, // Not Found: 121
0xff, // Not Found: 122
0xff, // Not Found: 123
0xff, // Not Found: 124
0xff, // Not Found: 125
0x3b, // 7e
0xff, // Not Found: 127
};

void Symbol6::writeReverse(ostream& str) {
    for (uint8_t a = 0; a < 128; a++) {
        // Try to find
        bool found = false;
        for (uint8_t s6 = 0; s6 < 64; s6++) {
            if (SCAMP6_TO_ASCII8[s6] == a) {
                str << "0x" << std::hex << (int)s6 << ", " << "// " << (int)a << std::dec << endl;
                found = true;
                break;
            }
        }
        if (!found) {
            str << "0xff" << ", " << "// Not Found: " << (int)a << endl;
        }
    }
}

Symbol6 Symbol6::ZERO(0);

Symbol6::Symbol6(uint8_t raw)
:   _raw(raw) {}

Symbol6 Symbol6::fromAscii(char asciiChar) {
    if (asciiChar >= 0) {
        return Symbol6(ASCII8_TO_SCAMP6[(uint8_t)asciiChar]);
    } else {
        return Symbol6(0b111111);
    }
}

uint8_t Symbol6::getRaw() const {
    return _raw;
}

char Symbol6::toAscii() const {
    if (_raw < 64) {
        uint8_t r = SCAMP6_TO_ASCII8[_raw];
        if (r == 0 || r == 0xff) {
            return ' ';
        } else {
            return r;
        }
    } else {
        return ' ';
    }
}
}

