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
#include <iomanip>

#include "Util.h"
#include "Symbol6.h"

using namespace std;

namespace scamp {

static const float PI = 3.1415926f;

void make_bar(ostream& str, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        str << "=";
    }
}

void render_spectrum(ostream& str, const cq15* x, uint16_t fftN, uint16_t sampleFreq) {
    // Get the max power
    const uint16_t b = max_idx(x, 1, (fftN / 2) - 1);
    const float b_max_power = x[b].mag_f32_squared();
    const unsigned int b_max_hz = (b * sampleFreq) / fftN;

    // Display a spectrum
    for (uint16_t i = 1; i < fftN / 2; i++) {
        const float frac = x[i].mag_f32_squared() / b_max_power;
        const unsigned int hz = (i * sampleFreq) / fftN;
        str << std::setw(4);
        str << i << " ";
        str << std::setw(4);
        str << hz << " |";
        make_bar(str, frac * 60.0);
        str << endl;
    }
}

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
    return used;
}

void visitTone(const unsigned int len, uint16_t sample_freq_hz, uint16_t tone_freq_hz,
    float amplitude, uint16_t phaseDegrees, std::function<void(unsigned int idx, float y)> cb) {

    float omega = 2.0f * PI * ((float)tone_freq_hz / (float)sample_freq_hz);
    float phi = 2.0f * PI * ((float)phaseDegrees / 360.0);
    float max_amp = 0;

    for (unsigned int i = 0; i < len; i++) {
        float sig = std::cos(phi) * amplitude;
        cb(i, sig);
        phi += omega;
    }
}

void addTone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] += f32_to_q15(x);
    };
    visitTone(len, sample_freq_hz, tone_freq_hz, amplitude, phaseDegrees, cb);
}

void make_tone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] = f32_to_q15(x);
    };
    visitTone(len, sample_freq_hz, tone_freq_hz, amplitude, phaseDegrees, cb);
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

void make_complex_tone_2(cq15* output, 
    uint32_t len, float bin, uint16_t binCount, 
    float amplitude, float phaseDegrees) {

    float omega = 2.0f * PI * bin / (float)binCount;
    float phi = 2.0f * PI * (phaseDegrees / 360.0);
    float max_amp = 0;

    for (uint32_t i = 0; i < len; i++) {
        float sig_i = std::cos(phi) * amplitude;
        output[i].r = f32_to_q15(sig_i);
        float sig_q = std::sin(phi) * amplitude;
        output[i].i = f32_to_q15(sig_q);
        phi += omega;
    }
}

float complex_corr(cq15* c0, cq15* c1, uint16_t len) {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(c0[i].r);
        float b = q15_to_f32(c0[i].i);
        float c = q15_to_f32(c1[i].r);
        // Complex conjugate
        float d = -q15_to_f32(c1[i].i);
        // Use the method that minimizes multiplication
        float ac = a * c;
        float bd = b * d;
        float a_plus_b = a + b;
        float c_plus_d = c + d;
        float p0 = a_plus_b * c_plus_d;
        result_r += (ac - bd) / (float)len;
        result_i += (p0 - ac - bd) / (float)len;
    }

    return std::sqrt(result_r * result_r + result_i * result_i);
}

// TODO: CLEAN THIS UP
float complex_corr_2(q15* c0, cq15* c1, uint16_t len) {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(c0[i]);
        float b = 0;
        float c = q15_to_f32(c1[i].r);
        // Complex conjugate
        float d = -q15_to_f32(c1[i].i);
        // Use the method that minimizes multiplication
        float ac = a * c;
        float bd = b * d;
        float a_plus_b = a + b;
        float c_plus_d = c + d;
        float p0 = a_plus_b * c_plus_d;
        result_r += (ac - bd) / (float)len;
        result_i += (p0 - ac - bd) / (float)len;
    }

    return std::sqrt(result_r * result_r + result_i * result_i);
}

} // namespace
