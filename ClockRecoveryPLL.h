#ifndef _ClockRecoveryPLL_H
#define _ClockRecoveryPLL_H

#include <cstdint>

namespace scamp {

    class ClockRecoveryPLL {
    public:

        ClockRecoveryPLL(unsigned int sampleRate);

        void setBitFrequencyHint(unsigned int hz);

        /**
         * This is the main function.  Call this at the sample rate frequency with 
         * an observation of the incoming signal.  If the return is true then the 
         * caller should use the sample as an observation of the curent bit.
        */
        bool processSample(bool mark);

        int getErrorIntegration() const { return _integration; }
        uint16_t getDCOPeriod() const { return _period; }

    private:

        void _edgeDetected();
        
        /**
         * Returns 1 or -1 depending on the phase of the DCO
        */
        int _getDCOValue() const;

        int16_t _integration;
        uint16_t _deltaPhi;
        uint16_t _phi;
        uint16_t _period;
        bool _samplePending;

        bool _lastMark;
    };
};

#endif