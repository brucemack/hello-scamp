/*
SCAMP Encoder/Decoder
Copyright (C) 2023 - Bruce MacKinnon KC1FSZ

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <iostream>
#include "ClockRecoveryPLL.h"

using namespace std;

namespace scamp {

ClockRecoveryPLL::ClockRecoveryPLL(unsigned int sampleRate) 
:   _idle(true),
    _sampleRate(sampleRate),
    _phi(0),
    // The target phase is 1/4 of the total range of uint16_t. We 
    // choose this phase so that 1/2 of the total range will fall
    // in the middle of a bit.
    _targetPhi((1L << 16) / 4),
    _omega(0),
    _Kp(7),
    _Ki(10),
    // Bias is based on the expected frequency
    // The initial bit frequency is set close to the SCAMP FSK rate
    // of 33.3 bits per second, or 60 samples per bit on a 2,000 kHz
    // sample clock.
    _bias((1L << 16) / 60),
    _integration(0),
    _lastPhi(0),
    _lastSample(false),
    _lastError(0),
    _samplesSinceEdge(0) {
}

void ClockRecoveryPLL::setBitFrequencyHint(unsigned int bitFrequency) {
    // Determine samples per bit
    uint16_t samplesPerBit = _sampleRate / bitFrequency;
    // Adjust the bias 
    _bias = (1L << 16) / samplesPerBit;
}

uint32_t ClockRecoveryPLL::getFrequency() const {   
    return (_sampleRate * (_omega + _bias)) >> 16;
}

bool ClockRecoveryPLL::processSample(bool mark) {

    int sample = mark ? 1 : -1;
    bool edge = false;
    
    // Look for the edge.  Only on edges do we adjust the phase.
    if (_lastSample != mark) {        
        _lastSample = mark;
        _samplesSinceEdge = 0;
        edge = true;
        // When coming in from idle, pretend like we are perfectly in sync to 
        // avoid a huge initial error.
        if (_idle) {
            _phi = _targetPhi;
            _idle = false;
        }
        // Compute the phase error
        _lastError = (int32_t)_targetPhi - (int32_t)_phi;
        // PI controller
        _integration += _lastError;
        // NOTICE: We are right-shifting here, so the coefficients are 
        // less than 1.0!
        _omega = (_lastError >> _Kp) + (_integration >> _Ki);        
    }

    // Keep rotating no matter what
    _phi += _omega;
    _phi += _bias;
    _samplesSinceEdge++;

    // Look for the rising edge of the two MSBs on _phi in order to 
    // detect the 270 degree phase point.
    bool phi180 = false;
    if ((_phi & 0x8000) && !(_lastPhi & 0x8000)) {
        phi180 = true;
    }
    _lastPhi = _phi;

    return phi180;
}

}
