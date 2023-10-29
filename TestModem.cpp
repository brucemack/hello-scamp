#include "TestModem.h"

namespace scamp {

TestModem::TestModem(int8_t* samples, unsigned int samplesSize, unsigned int samplesPerSymbol) 
:   _samples(samples),
    _samplesSize(samplesSize),
    _samplesUsed(0),
    _samplesPerSymbol(samplesPerSymbol) {
}

void TestModem::sendSilence() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) 
        _samples[_samplesUsed++] = 0;
}

void TestModem::sendMark() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) 
        _samples[_samplesUsed++] = 1;
}

void TestModem::sendSpace() {
    for (unsigned int i = 0; i < _samplesPerSymbol; i++) 
        _samples[_samplesUsed++] = -1;
}

}

