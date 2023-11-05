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

