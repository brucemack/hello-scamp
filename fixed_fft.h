#ifndef _fixed_fft_h
#define _fixed_fft_h

#include <cstdint>

//typedef int16_t fix15;
typedef int16_t q15;

#define q152float(a) ((float)(a) / 32768.0)
#define float2q15(a) ((q15)((a) * 32768.0)) 
#define multq15(a,b) ((q15)((((int32_t)(a))*((int32_t)(b)))>>15))

void fixed_fft_setup();
void fixed_fft(q15 fr[], q15 fi[]);

#endif
