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

cq15 cq15::mult(cq15 c0, cq15 c1) {
    q15 a = c0.r;
    q15 b = c0.i;
    q15 c = c1.r;
    q15 d = c1.i;
    // Use the method that minimizes multiplication
    q15 ac = mult_q15(a, c);
    q15 bd = mult_q15(b, d);
    q15 a_plus_b = a + b;
    q15 c_plus_d = c + d;
    q15 p0 = mult_q15(a_plus_b, c_plus_d);
    cq15 result;
    result.r = ac - bd;
    result.i = p0 - ac - bd;
    return result;
}

uint16_t max_idx(const cq15* sample, uint16_t start, uint16_t len) {
    float max_mag = 0;
    unsigned int max_bin = 0;
    for (unsigned int i = 0; i < len; i++) {
        if (i >= start) {
            float m = sample[i].mag_f32();
            if (m > max_mag) {
                max_mag = m;
                max_bin = i;
            }
        }
    }
    return max_bin;
}
