#include <cmath>
#include "FileModulator.h"

namespace scamp {

FileModulator::FileModulator(std::ostream& str, unsigned int sampleRate, unsigned int symbolLength, 
    unsigned int markFreq, unsigned int spaceFreq) 
:   _str(str), 
    _sampleRate(sampleRate),
    _symbolLength(symbolLength),
    _markFreq(markFreq),
    _spaceFreq(spaceFreq)
{
}

void FileModulator::sendMark() {
    _send(_markFreq);
}

void FileModulator::sendSpace() {
    _send(_spaceFreq);
}

void FileModulator::sendSilence() { 
    for (unsigned int i = 0; i < _symbolLength; i++) {
        _str << (int)0 << "\n";
    }
    _str.flush();
}

void FileModulator::_send(unsigned int freq) {

    const float scale = 32760;
    const float deltaPhi = 2.0 * 3.14159 * (float)freq / (float)_sampleRate;
    float phi = 0.0;

    for (unsigned int i = 0; i < _symbolLength; i++) {
        float s = std::cos(phi) * scale;
        _str << (int)s << "\n";
        phi += deltaPhi;
    }

    _str.flush();
}


}