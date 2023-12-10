/* 
Significant portions of this file are taken from Daniel Mark's
documentation that contains this copyright message:

Copyright (c) 2021 Daniel Marks

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "CodeWord24.h"
#include <cstdint>

namespace scamp {

// ===== Raw Golay Code Functions =============================================================


// 12x12 matrix 
static const uint16_t GOLAY_MATRIX[12] =
{
    0b110111000101,
    0b101110001011,
    0b011100010111,
    0b111000101101,
    0b110001011011,
    0b100010110111,
    0b000101101111,
    0b001011011101,
    0b010110111001,
    0b101101110001,
    0b011011100011,
    0b111111111110
};

/**
 * @param wd_enc The input is a 12-bit code word.  
 * @return The output is the 12-bits of parity.
 */
static uint16_t golay_mult(uint16_t wd_enc) {

    uint16_t enc = 0;
    uint8_t i;

    for (i = 12; i > 0;) {
        i--;
        if (wd_enc & 1) enc ^= GOLAY_MATRIX[i];
        wd_enc >>= 1;
    }
    return enc;
}

static uint8_t golay_hamming_weight_16(uint16_t n) {
    uint8_t s = 0, v;
    v = (n >> 8) & 0xFF;
    while (v)
    {
        v = v & (v - 1);
        s++;
    }
    v = n & 0xFF;
    while (v)
    {
        v = v & (v - 1);
        s++;
    }
    return s;
}

CodeWord24 CodeWord24::fromCodeWord12(CodeWord12 cw) {
    uint16_t rawCw12 = cw.getRaw();
    // Compute the parity for the data
    uint32_t parity = golay_mult(rawCw12);
    // Shift the 12 parity bits up to the MSB.  Data becomes LSB.
    uint32_t raw = (parity << 12) | rawCw12;
    return CodeWord24(raw);
}

CodeWord24::CodeWord24(uint32_t raw) 
:   _raw(raw) { }

uint32_t CodeWord24::getRaw() const {
    return _raw;
}

CodeWord12 CodeWord24::toCodeWord12() const {

    uint16_t enc = _raw & 0xFFF;
    uint16_t parity = _raw >> 12;
    uint8_t i;
    uint16_t syndrome, parity_syndrome;
    /* if there are three or fewer errors in the parity bits, then
    we hope that there are no errors in the data bits, otherwise
    the error is uncorrected */
    syndrome = golay_mult(enc) ^ parity;
    uint8_t biterr = golay_hamming_weight_16(syndrome);
    if (biterr <= 3) {
        return CodeWord12(enc, true);
    }
    /* check to see if the parity bits have no errors */
    parity_syndrome = golay_mult(parity) ^ enc;
    biterr = golay_hamming_weight_16(parity_syndrome);
    if (biterr <= 3) {
        return CodeWord12(enc ^ parity_syndrome, true);
    }
    /* we flip each bit of the data to see if we have two or fewer errors */
    for (i = 12; i > 0;) {
        i--;
        biterr = golay_hamming_weight_16(syndrome ^ GOLAY_MATRIX[i]);
        if (biterr <= 2)
        {
            return CodeWord12(enc ^ (((uint16_t)0x800) >> i), true);
        }
    }
    /* we flip each bit of the parity to see if we have two or fewer errors */
    for (i = 12; i > 0;) {
        i--;
        uint16_t par_bit_synd = parity_syndrome ^ GOLAY_MATRIX[i];
        biterr = golay_hamming_weight_16(par_bit_synd);
        if (biterr <= 2)
        {
            return CodeWord12(enc ^ par_bit_synd, true);
        }
    }
    /* uncorrectable error */
    return CodeWord12(0xFFFF, false); 
}

}
