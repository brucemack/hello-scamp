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
#include "Frame30.h"

// Masks used to pull data out of a 24 bit value
#define MASK24_4_5 0b111100000000000000000000
#define MASK24_4_4 0b000011110000000000000000
#define MASK24_4_3 0b000000001111000000000000
#define MASK24_4_2 0b000000000000111100000000
#define MASK24_4_1 0b000000000000000011110000
#define MASK24_4_0 0b000000000000000000001111

#define MASK30_4_5 0b011110000000000000000000000000  // Shift down 25
#define MASK30_4_4 0b000000111100000000000000000000  // Shift down 20
#define MASK30_4_3 0b000000000001111000000000000000  // Shift down 15
#define MASK30_4_2 0b000000000000000011110000000000  // Shift down 10
#define MASK30_4_1 0b000000000000000000000111100000  // Shift down  5
#define MASK30_4_0 0b000000000000000000000000001111  // Shift down  0

#define MASK30_HIGH 0b100000000000000000000000000000

using namespace std;

namespace scamp {

/**
 * @param data_4 4-bit int
 * @return A 5-bit where bit 4 is the compliment of bit 3
*/
uint8_t add_comp(uint8_t data_4) {
    if (data_4 & 0b1000) {
        return data_4;
    } else {
        return 0b10000 | data_4;
    }
}

Frame30 Frame30::fromCodeWord24(CodeWord24 cw) {
    uint32_t codeword_24 = cw.getRaw();
    // Pull out the 4-bit sections from the codeword
    uint8_t d5_4 = (codeword_24 & MASK24_4_5) >> 20;
    uint8_t d4_4 = (codeword_24 & MASK24_4_4) >> 16;
    uint8_t d3_4 = (codeword_24 & MASK24_4_3) >> 12;
    uint8_t d2_4 = (codeword_24 & MASK24_4_2) >> 8;
    uint8_t d1_4 = (codeword_24 & MASK24_4_1) >> 4;
    uint8_t d0_4 = (codeword_24 & MASK24_4_0);
    // Create the compliments a build up the packet
    uint32_t raw = (add_comp(d5_4) << 25) |
        (add_comp(d4_4) << 20) |
        (add_comp(d3_4) << 15) |
        (add_comp(d2_4) << 10) |
        (add_comp(d1_4) << 5) |
        (add_comp(d0_4));
    return Frame30(raw);
}

Frame30 Frame30::fromTwoAsciiChars(char a, char b) {
    Symbol6 sym0 = Symbol6::fromAscii(a);
    Symbol6 sym1 = Symbol6::fromAscii(b);
    CodeWord12 cw12 = CodeWord12::fromSymbols(sym0, sym1);
    CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
    return Frame30::fromCodeWord24(cw24);
}

/**
 * Converts an array of tones to a 30-bit integer using the LSBs.
 * The MSB is the first to be sent. a[0] is the first to be sent.
 */
uint32_t Frame30::arrayToInt32(uint8_t a[]) {
    uint32_t result;
    for (unsigned int i = 0; i < 30; i++) {
        if (a[i] == 1) {
            result = result | 1;
        }
        result <<= 1;
    }
    return result;
}

/**
 * Utility function that decodes an array of marks/spaces
 * an creates a frame.  The first tone received is in 
 * the [0] location and the last tone received is in the 
 * [29] location.
 */
Frame30 Frame30::decodeFromTones(uint8_t tones[]) {
    return Frame30(arrayToInt32(tones));
}

Frame30 Frame30::ZERO_FRAME(0b000000000000000000000000000000);
Frame30 Frame30::ALT_FRAME (0b101010101010101010101010101010);
Frame30 Frame30::START_FRAME(0b111111111111111111111111010101);
Frame30 Frame30::SYNC_FRAME(0b111110110100011001110100011110);

uint32_t Frame30::MASK30LSB = 0b00111111111111111111111111111111;

Frame30::Frame30() 
:   _raw(0) { }

Frame30::Frame30(uint32_t raw) 
:   _raw(raw) { }

uint32_t Frame30::getRaw() const {
    return _raw;
}

// MSB is transmitted first!
void Frame30::transmit(Modulator& mod, uint32_t symbolDurationUs, int16_t errorMs) {
    uint32_t work = _raw;
    for (unsigned int i = 0; i < 30; i++) {
        if (work & MASK30_HIGH) {
            mod.sendMark(symbolDurationUs);
        } else {
            mod.sendSpace(symbolDurationUs);
        }
        work <<= 1;
    }
}

bool Frame30::isValid() const {
    return getComplimentCount() == 6;
}

unsigned int Frame30::getComplimentCount() const {
    unsigned int result = 0;
    uint32_t work = _raw;
    // There are 6 sets of compliments
    for (unsigned int i = 0; i < 6; i++) {
        // Detect compliment with C bit and teh data bit immediately following
        if ((work & MASK30_HIGH) != ((work << 1) & MASK30_HIGH)) {
            result++;
        }
        work <<= 5;
    }
    return result;
}

int32_t Frame30::correlate30(uint32_t w0, uint32_t w1) {
    int32_t result = 0;
    for (unsigned int i = 0; i < 30; i++) {
        int i0 = (w0 & 1) ? 1 : -1;
        int i1 = (w1 & 1) ? 1 : -1;
        w0 = w0 >> 1;
        w1 = w1 >> 1;
        result += (i0 * i1);
    }
    return result;
}

CodeWord24 Frame30::toCodeWord24() const {   
    // Pull out the 4-bit sections from the frame
    uint8_t d5_4 = (_raw & MASK30_4_5) >> 25;
    uint8_t d4_4 = (_raw & MASK30_4_4) >> 20;
    uint8_t d3_4 = (_raw & MASK30_4_3) >> 15;
    uint8_t d2_4 = (_raw & MASK30_4_2) >> 10;
    uint8_t d1_4 = (_raw & MASK30_4_1) >> 5;
    uint8_t d0_4 = (_raw & MASK30_4_0);
    // Build the result
    return CodeWord24((d5_4 << 20) |
           (d4_4 << 16) |
           (d3_4 << 12) |
           (d2_4 << 8) |
           (d1_4 << 4) |
           (d0_4));
}

}