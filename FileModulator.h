#ifndef _FileModulator_h
#define _FileModulator_h

#include <iostream>
#include "Modulator.h"

namespace scamp {

    class FileModulator : public Modulator {
    public:

        FileModulator(std::ostream& str, unsigned int sampleRate, unsigned int symbolLength, 
            unsigned int markFreq, unsigned int spaceFreq);

        void sendMark();
        void sendSpace();

    private:

        void _send(unsigned int freq);

        std::ostream& _str;
        unsigned int _sampleRate;
        unsigned int _symbolLength;
        unsigned int _markFreq;
        unsigned int _spaceFreq;
    };

}

#endif

