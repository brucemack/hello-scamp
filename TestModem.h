#ifndef _TestModem_h
#define _TestModem_h

#include <cstdint>
#include "Modulator.h"

namespace scamp {

class TestModem : public Modulator {
public:

    TestModem(int8_t* samples, unsigned int tonesSize, unsigned int samplesPerSymbol);

    virtual void sendSilence();
    virtual void sendMark();
    virtual void sendSpace();

    unsigned int getSamplesUsed() const { return _samplesUsed; }

private:

    int8_t* _samples;
    unsigned int _samplesSize;
    unsigned int _samplesUsed;
    unsigned int _samplesPerSymbol;
};

}

#endif
