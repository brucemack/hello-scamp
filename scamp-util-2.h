#ifndef _scamp_util_2_h
#define _scamp_util_2_h

#include <cstdint>

namespace kc1fsz {
    namespace scamp { 

        /**
         * @return 30 bit frame
         */
        uint32_t make_frame(uint32_t codeword_24);

        /**
         * @return 24 bit codeword
        */
        uint32_t frame_to_codeword24(uint32_t frame30);

        /** 
         * Correlates the first 30 bits of the 32 bit number
        */
        int32_t correlate(uint32_t w0, uint32_t w1);

        uint8_t ascii2scamp(uint8_t scamp_code);
        uint8_t scamp2ascii(uint8_t ascii_code);
    }
}

#endif
