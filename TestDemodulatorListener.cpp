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

void TestDemodulatorListener::dataSyncAcquired() {
    cout << "Data Sync Acquired" << endl;
}

void TestDemodulatorListener::frequencyLocked(uint16_t markFreq, 
    uint16_t spaceFreq) {
    cout << "Frequency locked at mark=" << markFreq << ", space=" << spaceFreq << endl;
}

void TestDemodulatorListener::badFrameReceived(uint32_t rawFrame) {
    cout << "Bad frame ignored" << endl;
}

void TestDemodulatorListener::received(char asciiChar) {
    _out << asciiChar;
}

string TestDemodulatorListener::getMessage() const {
    return _out.str();
}

}
