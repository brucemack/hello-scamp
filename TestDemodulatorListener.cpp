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
#include "TestDemodulatorListener.h"

using namespace std;

namespace scamp {

TestDemodulatorListener::TestDemodulatorListener(ostream& out) 
:   _out(out),
    _sampleSpace(0),
    _sampleSpaceSize(0) {
}

TestDemodulatorListener::TestDemodulatorListener(ostream& out,
    Sample* sampleSpace, uint16_t sampleSpaceSize) 
:   _out(out),
    _sampleSpace(sampleSpace),
    _sampleSpaceSize(sampleSpaceSize) {
}

void TestDemodulatorListener::dataSyncAcquired() {
    _out << "Data Sync Acquired" << endl;
}

void TestDemodulatorListener::frequencyLocked(uint16_t markFreq, 
    uint16_t spaceFreq) {
    _out << "Frequency locked at mark=" << markFreq << ", space=" << spaceFreq << endl;
    // Check to see if we should trigger
    if (_triggerMode == ON_LOCK) {
        _triggered = true;
        _delayCounter = _triggerDelay;
    }
}

void TestDemodulatorListener::badFrameReceived(uint32_t rawFrame) {
    _out << "Bad frame ignored" << endl;
}

void TestDemodulatorListener::received(char asciiChar) {
    _msg << asciiChar;
    //_out << "CHAR: " << asciiChar << endl;
}

string TestDemodulatorListener::getMessage() const {
    return _msg.str();
}

void TestDemodulatorListener::goodFrameReceived() {
}

void TestDemodulatorListener::sampleMetrics(uint8_t activeSymbol, bool capture, 
   int32_t pllError,
   float* symbolCorr, float* symbolCorrAvg, float maxCorr) {

    //if (capture) {
    //    cout << pllError << endl;
    //}

    if (_triggered) {
        if (_delayCounter > 0) {
            _delayCounter--;
        } else {
            if (_sampleSpacePtr < _sampleSpaceSize) {
                // Make a sample
                Sample* s = &(_sampleSpace[_sampleSpacePtr]);
                s->activeSymbol = activeSymbol;
                s->capture = capture;
                s->pllError = pllError;
                s->symbolCorr[0] = symbolCorr[0];
                s->symbolCorr[1] = symbolCorr[1];
                s->symbolCorrAvg[0] = symbolCorrAvg[0];
                s->symbolCorrAvg[1] = symbolCorrAvg[1];
                s->maxCorr = maxCorr;
                _sampleSpacePtr++;
            }
        }
    }
}

void TestDemodulatorListener::bitTransitionDetected() {
}

void TestDemodulatorListener::receivedBit(bool bit, uint16_t frameBitPos, 
    int syncFrameCorr) {
    //if (frameBitPos == 0) {
    //    _out << "====" << endl;
    //}
    //_out << "BIT (" << frameBitPos << ") = " << (int)bit 
    //    << ", c=" << syncFrameCorr << endl;
}

void TestDemodulatorListener::setTriggerMode(TriggerMode mode) {
    _triggerMode = mode;
}

void TestDemodulatorListener::setTriggerDelay(uint16_t d) {
    _triggerDelay = d;
}

void TestDemodulatorListener::setTriggered(bool t) {
    _triggered = t;
    if (_triggered) {
        _delayCounter = _triggerDelay;
        _sampleSpacePtr = 0;
    }
}

void TestDemodulatorListener::clearSamples() {
    _sampleSpacePtr = 0;
}

void TestDemodulatorListener::dumpSamples(std::ostream& str) const {
    for (uint16_t i = 0; i < _sampleSpacePtr; i++) {
        if (i % 60 == 0) {
            str << "--------" << endl;
        }
        Sample* s = &(_sampleSpace[i]);
        str << i << " " 
            << i / 60 << " "
            << (int)s->activeSymbol << " " 
            << (int)s->capture << " " 
            << s->pllError << " " 
            << s->maxCorr << " " 
            << s->symbolCorr[1] << " " 
            << s->symbolCorr[0] << " " 
            << s->symbolCorrAvg[1] << " " 
            << s->symbolCorrAvg[0] << " " 
            << (int)(100.0 * (s->symbolCorrAvg[1] / s->maxCorr)) << " " 
            << (int)(100.0 * (s->symbolCorrAvg[0] / s->maxCorr)) << " " 
            << endl;
    }
}

}