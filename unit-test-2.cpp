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
#include <cmath>
#include "ClockRecoveryPLL.h"

using namespace std;
using namespace scamp;

int main(int argc, const char** argv) {

    ClockRecoveryPLL pll(2000);
    pll.setBitFrequencyHint(50);

    int bits[8] = { 1, 0, 1, 0, 1, 0, 1, 0 };

    cout << "TIME" << "," << "ERROR" << "," << "INT" << "," << "FREQ" << endl;

    uint16_t t = 0;
    // Repeats
    for (unsigned int k = 0; k < 16; k++) {
        // Bit loop
        for (unsigned int bit = 0; bit < 8; bit++) {
            // Oversampling loop
            for (unsigned int sample = 0; sample < 60; sample++) {
                bool mark = bits[bit] ? true : false;
                bool edge = pll.processSample(mark);
                cout << t << "," << pll.getLastError() << "," << pll.getIntegration() << "," << 
                    pll.getFrequency() << "," << edge << endl;
                t++;
            }
        }
    }
}

int main2(int argc, const char** argv) {

    ClockRecoveryPLL pll(2000);

    // For this dataset the bit rate is 1 bit for every 8 samples.
    const int data[] = { 0,0,0,0,0,0,0,0,
                   1,1,1,1,1,1,1,1,
                   1,1,1,1,1,1,1,1,
                   0,0,0,0,0,0,0,0,
                   1,1,1,1,1,1,1,1,
                   0,0,0,0,0,0,0,0  };

    cout << "TIME" << "," << "ERROR" << endl;
    uint16_t t = 0;
    for (int k = 0; k < 32; k++) {
        for (int i = 0; i < 48; i++) {
            bool mark = data[i] ? true : false;
            pll.processSample(mark);
            cout << t << "," << pll.getLastError() << endl;
            t++;
        }
    }

    return 0;
}
