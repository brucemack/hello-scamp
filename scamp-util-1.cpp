/**
 * Code copied from:
 * 
 * Simple Conversational Amateur Messaging Protocol (SCAMP)
 * by Daniel Marks, KW4TI
 */
#include <iostream>
#include <cstdint>
#include "scamp-util-1.h"

namespace kw4ti {

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
uint16_t golay_mult(uint16_t wd_enc) {

    uint16_t enc = 0;
    uint8_t i;

    for (i = 12; i > 0;) {
        i--;
        if (wd_enc & 1) enc ^= GOLAY_MATRIX[i];
        wd_enc >>= 1;
    }
    return enc;
}

uint8_t golay_hamming_weight_16(uint16_t n)
{
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

/**
 * Encoding
 *
 * Text:
 *   2 6-bit characters (YYYYYY XXXXXX) concatenated into 12 data bits.  XXXXXX is first in stream.
 *   12 data bits -> 24 bits (with parity) -> 30 bit packet (with compliments)
 * 
 * wd_enc The data payload (12 bits)
 * @return The code word (24 bit)
 */
scampCodeWord24_t golay_encode(scampCodeWord12_t wd_enc_12)
{
    // Compute the parity for the data
    scampCodeWord24_t parity = golay_mult(wd_enc_12);
    // Shift the 12 parity bits up to the MSB.  Data becomes LSB.
    return (((scampCodeWord24_t)parity) << 12) | wd_enc_12;
}

/**
 * Decoder.  
 * @param codeword_24 The codeword to be decoded.  12 bits of parity and 12 bits of data.
 * @param biterrs The number of bit errors corrected.
 * @return The data payload (12 bits), or 0xffff if there are too many errors to correct.
 */
scampCodeWord12_t golay_decode(scampCodeWord24_t codeword_24, uint8_t *biterrs)
{
    uint16_t enc = codeword_24 & 0xFFF;
    uint16_t parity = codeword_24 >> 12;
    uint8_t i, biterr;
    uint16_t syndrome, parity_syndrome;
    /* if there are three or fewer errors in the parity bits, then
    we hope that there are no errors in the data bits, otherwise
    the error is uncorrected */
    syndrome = golay_mult(enc) ^ parity;
    biterr = golay_hamming_weight_16(syndrome);
    if (biterr <= 3)
    {
        *biterrs = biterr;
        return enc;
    }
    /* check to see if the parity bits have no errors */
    parity_syndrome = golay_mult(parity) ^ enc;
    biterr = golay_hamming_weight_16(parity_syndrome);
    if (biterr <= 3)
    {
        *biterrs = biterr;
        return enc ^ parity_syndrome;
    }
    /* we flip each bit of the data to see if we have two or fewer errors */
    for (i=12;i>0;)
    {
        i--;
        biterr = golay_hamming_weight_16(syndrome ^ GOLAY_MATRIX[i]);
        if (biterr <= 2)
        {
            *biterrs = biterr+1;
            return enc ^ (((uint16_t)0x800) >> i);
        }
    }
    /* we flip each bit of the parity to see if we have two or fewer errors */
    for (i=12;i>0;)
    {
        i--;
        uint16_t par_bit_synd = parity_syndrome ^ GOLAY_MATRIX[i];
        biterr = golay_hamming_weight_16(par_bit_synd);
        if (biterr <= 2)
        {
            *biterrs = biterr+1;
            return enc ^ par_bit_synd;
        }
    }
    return 0xFFFF; /* uncorrectable error */
}

}
