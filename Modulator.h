#ifndef _Modulator_h
#define _Modulator_h

namespace scamp {
    class Modulator {
    public:

        virtual void sendSilence() { }
        virtual void sendMark() = 0;
        virtual void sendSpace() = 0;
    };
}

#endif 
