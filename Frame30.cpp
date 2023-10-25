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

Frame30 Frame30::ZERO_FRAME(0b000000000000000000000000000000);
Frame30 Frame30::SYNC_FRAME_0(0b111111111111111111111111010101);
Frame30 Frame30::SYNC_FRAME_1(0b111110110100011001110100011110);

Frame30::Frame30(uint32_t raw) 
:   _raw(raw) { }

uint32_t Frame30::getRaw() const {
    return _raw;
}

// MSB is transmitted first!
void Frame30::transmit(Modulator& mod) {
    uint32_t work = _raw;
    for (unsigned int i = 0; i < 30; i++) {
        if (work & MASK30_HIGH) {
            mod.sendMark();
        } else {
            mod.sendSpace();
        }
        work <<= 1;
    }
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

}