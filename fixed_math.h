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
#ifndef _fixed_math_h
#define _fixed_math_h

#include <cstdint>

typedef int16_t q15;

struct complex_q15 {
    q15 r;
    q15 i;
};

// For multiplication we use a 32-bit result to avoid loss of the fraction
#define mult_q15(a,b) ( (q15)((((int32_t)(a)) * ((int32_t)(b))) >> 15) )

#define abs_q15(a) abs(a) 

#define q15_to_float(a) ((float)(a) / 32768.0)
#define float_to_q15(a) ((q15)((a) * 32768.0)) 
#define int_to_q15(a) ((q15)(a << 15))
#define q15_to_int(a) ((int)(a >> 15))
#define char_to_q15(a) (q15)(((q15)(a)) << 15)

#endif
