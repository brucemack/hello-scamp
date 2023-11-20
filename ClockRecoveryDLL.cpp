
#include "ClockRecoveryDLL.h"

namespace scamp {

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
    // Move forward and wrap
    _phi = (_phi + _omega) & 0x7fff;
    // When the wrap happens the phi suddenly becomes smaller
    bool capture = _phi < _lastPhi;
    _lastPhi = _phi;
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
    uint16_t error = _phi - _targetPhi;
    _lastError = error;

    // Apply the gain
    error >>= 1;
    _phi += error;
    _samplesSinceEdge = 0;
}

}
