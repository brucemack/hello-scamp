#include <iostream>
#include <cstdint>

#include "scamp-util-2.h"

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

scampFrame30_t make_frame(scampCodeWord24_t codeword_24) {
    
    // Pull out the 4-bit sections from the codeword
    uint8_t d5_4 = (codeword_24 & MASK24_4_5) >> 20;
    uint8_t d4_4 = (codeword_24 & MASK24_4_4) >> 16;
    uint8_t d3_4 = (codeword_24 & MASK24_4_3) >> 12;
    uint8_t d2_4 = (codeword_24 & MASK24_4_2) >> 8;
    uint8_t d1_4 = (codeword_24 & MASK24_4_1) >> 4;
    uint8_t d0_4 = (codeword_24 & MASK24_4_0);
    // Create the compliments a build up the packet
    return (add_comp(d5_4) << 25) |
           (add_comp(d4_4) << 20) |
           (add_comp(d3_4) << 15) |
           (add_comp(d2_4) << 10) |
           (add_comp(d1_4) << 5) |
           (add_comp(d0_4));

    return 0;
}

scampCodeWord24_t frame_to_codeword24(scampFrame30_t frame30) {
    
    // Pull out the 4-bit sections from the frame
    uint8_t d5_4 = (frame30 & MASK30_4_5) >> 25;
    uint8_t d4_4 = (frame30 & MASK30_4_4) >> 20;
    uint8_t d3_4 = (frame30 & MASK30_4_3) >> 15;
    uint8_t d2_4 = (frame30 & MASK30_4_2) >> 10;
    uint8_t d1_4 = (frame30 & MASK30_4_1) >> 5;
    uint8_t d0_4 = (frame30 & MASK30_4_0);
    // Build the result
    return (d5_4 << 20) |
           (d4_4 << 16) |
           (d3_4 << 12) |
           (d2_4 << 8) |
           (d1_4 << 4) |
           (d0_4);
}

int32_t correlate(scampFrame30_t w0, scampFrame30_t w1) {
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
}

