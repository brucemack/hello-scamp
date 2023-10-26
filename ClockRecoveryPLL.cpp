#include <iostream>
#include "ClockRecoveryPLL.h"

using namespace std;

namespace scamp {

ClockRecoveryPLL::ClockRecoveryPLL(unsigned int sampleRate) 
:   _phi(0),
    _deltaPhi(1),
    _period(8),
    _samplePending(false),
    _integration(0) {
}

bool ClockRecoveryPLL::processSample(bool mark) {

    int sample = (mark) ? 1 : -1;
    //cout << "Sample: " << sample << ", DCO: " << _getDCOValue() << endl;
    
    // Look for the edge
    if (_lastMark != mark) {

        _lastMark = mark;

        // Deal with error integration
        int error = (sample == _getDCOValue()) ? 1 : -1;
        _integration -= error;

        //cout << "  ERR=" << error << ", INT=" << _integration << endl;

        // Speed up or slow down
        _period += _integration;
        if ((int)_period < 2) {
            _period = 2;
        }   
    }

    // Advance the DCO
    _phi += _deltaPhi;
    // Process the wrap if needed
    if (_phi >= _period) {
        _phi = _phi % _period;
        _samplePending = true;
    }

    if (_samplePending) {
        _samplePending = false;
        return true;
    } else {
        return false;
    }
}

int ClockRecoveryPLL::_getDCOValue() const {
    if (_phi > (_period >> 1)) {
        return -1;
    } else {
        return 1;
    }
}

}
