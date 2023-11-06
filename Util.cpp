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
#include "Util.h"
#include "Symbol6.h"

namespace scamp {

static const float PI = 3.1415926f;

void makePairs(const char* in, std::function<void(char a, char b)> cb) {
    const char* p = in;
    while (*p != 0) {
        char a = *p;
        p++;
        char b;
        if (*p == 0) {
            b = 0;
        } else {
            b = *p;
            p++;
        }
        cb(a, b);
    }
}

unsigned int encodeString(const char* in, Frame30* outList, unsigned int outListSize, 
    bool includeSyncFrame) {

    unsigned int used = 0;

    if (includeSyncFrame) {
        if (used < outListSize) {
            outList[used++] = Frame30::START_FRAME;
        }
        if (used < outListSize) {
            outList[used++] = Frame30::SYNC_FRAME;
        }
    }

    // Make a Lambda that takes the next two characters in the input and 
    // adds a frame to the output list.
    makePairs(in, 
        //std::function<void(char a, char b)> procPair = 
        [&used, outList, outListSize](char a, char b) {  
            if (used < outListSize) {
                outList[used++] = Frame30::fromTwoAsciiChars(a, b);
            }
        }
    );
    // Trigger the frame building
    //makePairs(in, procPair);
    return used;
}

void make_tone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {

    float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
    float phi = 2.0f * PI * (phaseDegrees / 360.0);
    float max_amp = 0;

    for (unsigned int i = 0; i < len; i++) {
        float sig = std::cos(phi) * amplitude;
        output[i] = f32_to_q15(sig);
        phi += omega;
    }
}

void make_complex_tone(cq15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {

    float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
    float phi = 2.0f * PI * (phaseDegrees / 360.0);
    float max_amp = 0;

    for (unsigned int i = 0; i < len; i++) {
        float sig_i = std::cos(phi) * amplitude;
        output[i].r = f32_to_q15(sig_i);
        float sig_q = std::sin(phi) * amplitude;
        output[i].i = f32_to_q15(sig_q);
        phi += omega;
    }
}

} // namespace

