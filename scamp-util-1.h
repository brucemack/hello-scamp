#ifndef _scamp_util_1_h
#define _scamp_util_1_h

#include <cstdint>

namespace kw4ti {
    uint16_t golay_mult(uint16_t wd_enc);
    /**
     * Converts a 12-bit data/text word into a 24-bit Golay code word
    */
    uint32_t golay_encode(uint16_t wd_enc_12);
    uint16_t golay_decode(uint32_t codeword_24, uint8_t *biterrs);
}

#endif
