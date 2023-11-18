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
#ifndef _Util_h
#define _Util_h

#include <iostream>
#include <functional>
#include <cinttypes>

#include "Frame30.h"
#include "fixed_math.h"
#include "Modulator.h"

namespace scamp {

/**
 * Takes a string and returns it in char pairs.
*/
void makePairs(const char* in, std::function<void(char a, char b)> cb);

/**
 * @returns The number of frames written into the list
*/
unsigned int encodeString(const char* in, Frame30* outList, unsigned int outListSize, 
    bool includeSyncFrame);

void visitTone(const unsigned int len, uint16_t sample_freq_hz, uint16_t tone_freq_hz,
    float amplitude, uint16_t phaseDegrees, std::function<void(uint16_t idx, float y)> cb);

void make_real_tone(q15* output, const unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees = 0);

void make_real_tone_distorted(q15* output, const unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees, float dcOffset);

void addTone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees = 0);

void make_complex_tone(cq15* output, unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees = 0);

float complex_corr(cq15* c0, cq15* c1, uint16_t len);

float corr_real_complex(const q15* c0, const cq15* c1, uint16_t len);

/**
 * This is a convolution function that supports a circular buffer
 * for the c0 (real) series and a linear buffer for the c1 (complex)
 * series. This would typically be used to convolve input samples
 * from an ADC with the complex coefficients of an FIR or quadrature
 * demodulator.
 * 
 * Automatic wrapping on the c0 buffer is used to avoid going off the end.
 */
float corr_real_complex_2(const q15* c0, uint16_t c0Base, uint16_t c0Size, 
    const cq15* c1, uint16_t c1Len);

float pi();


void make_bar(std::ostream& str, unsigned int len);

/**
 * A utility function for dealing with circular buffers.  Result
 * is (base + disp) % size.
*/
uint16_t wrapIndex(uint16_t base, uint16_t disp, uint16_t size);

void render_spectrum(std::ostream& str, const cq15* x, uint16_t fftN, uint16_t sampleFreq);

/**
 * Takes a null-temrinated ASCII message, encodes it, and transmits
 * it using the modulator provided.
 */
uint16_t modulateMessage(const char* asciiMsg, Modulator& mod,  
    Frame30* frames, const uint16_t maxFrames);
}

#endif
