#ifndef _scamp_util_1_h
#define _scamp_util_1_h

#include <cstdint>
#include "Util.h"

namespace kw4ti {

    uint16_t golay_mult(uint16_t wd_enc);

    /**
     * Converts a 12-bit data/text word into a 24-bit Golay code word
    */
    scampCodeWord24_t golay_encode(scampCodeWord12_t codeword_12);

    scampCodeWord12_t golay_decode(scampCodeWord24_t codeword_24, uint8_t *biterrs);
}

#endif
