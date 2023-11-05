#include "fixed_math.h"

q15 corr_q15(q15* data, q15* carrier, uint16_t len) {
    const uint16_t shift = std::log2(len);
    q15 result = 0;
    for (uint16_t i = 0; i < len; i++) {
        q15 p = mult_q15(data[i], carrier[i]);
        // To prevent overflow we shift down
        result += (p >> shift);
    }
    return result;
}

