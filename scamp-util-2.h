#ifndef _scamp_util_2_h
#define _scamp_util_2_h

#include <cstdint>
#include "Util.h"

namespace scamp { 

    /**
     * @return 30 bit frame
     */
    scampFrame30_t make_frame(scampCodeWord24_t codeword_24);

    /**
     * @return 24 bit codeword
    */
    scampCodeWord24_t frame_to_codeword24(scampFrame30_t frame30);

    /** 
     * Correlates the first 30 bits of the 32 bit number
    */
    int32_t correlate(scampFrame30_t w0, scampFrame30_t w1);

    uint8_t ascii2scamp(uint8_t scamp_code);
    uint8_t scamp2ascii(uint8_t ascii_code);
}

#endif
