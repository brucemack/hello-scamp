#include <iostream>
#include "ClockRecoveryDLL.h"

using namespace std;

namespace scamp {

ClockRecoveryDLL::ClockRecoveryDLL(uint16_t sampleRate) 
:   _sampleRate(sampleRate) {
}

void ClockRecoveryDLL::setClockFrequency(uint16_t dataFreq) {    
    uint32_t y = _sampleRate / dataFreq;
    _omega = (uint32_t)_maxPhi / y;
}

bool ClockRecoveryDLL::processSample(uint8_t symbol) {

    if (_lastSymbol != symbol) {
        _edgeDetected();
        _lastSymbol = symbol;
    }
    _samplesSinceEdge++;
    // Look for tht wrap-around
    bool capture = (int32_t)_phi + (int32_t)_omega > (int32_t)_maxPhi;
    // Move forward and wrap
    _phi = (_phi + _omega) & 0x7fff;
    _lastPhi = _phi;
    return capture;
}

int16_t ClockRecoveryDLL::getLastError() const { 
    return _lastError; 
}

float ClockRecoveryDLL::getLastPhaseError() const {
    return (float) _lastError / (float)_maxPhi;
}

uint32_t ClockRecoveryDLL::getClockFrequency() const {
    return (_omega * _sampleRate) / _maxPhi;
}

uint16_t ClockRecoveryDLL::getSamplesSinceEdge() const { 
    return _samplesSinceEdge; 
}

void ClockRecoveryDLL::_edgeDetected() {  
    // Error will be positive if we are lagging the target phase  
    int16_t error = _phi - _targetPhi;
    _lastError = error;
    _errorIntegration += error;

    // Apply the gain
    //int32_t adj = (error >> 1) + (error >> 2);
    int32_t adj = (error >> 1);

    /*
    cout << "_phi=" << _phi << ", _targetPhi=" << _targetPhi << ", omega=" << _omega 
        << ", error=" << error 
        << ", adj=" << -adj 
        << ", int=" << _errorIntegration 
        << endl;
    */

    _phi -= adj;
    _samplesSinceEdge = 0;
}

}
