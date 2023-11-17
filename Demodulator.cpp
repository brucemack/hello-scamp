/* 
Significant portions of this file are taken from Daniel Mark's
documentation that contains this copyright message:

Copyright (c) 2021 Daniel Marks

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include <cstdint>

#include "Util.h"
#include "Demodulator.h"

namespace scamp {

Demodulator::Demodulator(DemodulatorListener* listener, 
    uint16_t sampleFreq, uint16_t fftN, q15* fftTrigTable, q15* fftWindow,
    q15* bufferSpace)
:   _listener(listener),
    _sampleFreq(sampleFreq),
    _fftN(fftN),
    _fft(fftN, fftTrigTable),
    _fftWindow(fftWindow),
    _pll(sampleFreq),
    _buffer(bufferSpace) { 

    // NOTICE: We are purposely setting the initial frequency slightly 
    // wrong to show that PLL will adjust accordingly.
    _pll.setBitFrequencyHint(36);

    // Build the Hann window for the FFT (raised cosine)
    for (uint16_t i = 0; i < _fftN; i++) {
        _fftWindow[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)_fftN))));
    }
}

}

