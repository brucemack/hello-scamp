
#include "ClockRecoveryDLL.h"

ClockRecoveryDLL::ClockRecoveryDLL(uint16_t sampleRate) 
:   _sampleRate(sampleRate) {
}

void ClockRecoveryDLL::setDataFrequency(uint16_t dataFreq) {    
    uint32_t t = (_maxPhi * dataFreq) / _sampleRate;
    _omega = t;
}

bool ClockRecoveryDLL::processSample(uint8_t symbol) {

    if (_lastSymbol != symbol) {
        _edgeDetected();
        _lastSymbol = symbol;
    }

    _samplesSinceEdge++;
    _phi += _omega;
    // See if we just passed the half mark
    bool capture = (_lastPhi <= _halfPhi) && (_phi > _halfPhi);
    _lastPhi = phi;
    return capture;
}

int16_t ClockRecoveryDLL::getLastError() const { 
    return _lastError; 
}

uint32_t ClockRecoveryDLL::getDataFrequency() const {
    return (_omega * _sampleRate) / _maxPhi;
}

uint16_t ClockRecoveryDLL::getSamplesSinceEdge() const { 
    return _samplesSinceEdge; 
}

void ClockRecoveryDLL::_edgeDetected() {    
    uint16_t error = 0;
    // If we're in the second half of the phase then the error is negative
    if (_phi > (_maxPhi >> 1)) {
        error = _phi - _maxPhi;
    } else {
        error = _phi;
    }
    _lastError = error;
    // Apply the gain
    error >>= 1;

    _phi += error;
    _samplesSinceEdge = 0;
}
