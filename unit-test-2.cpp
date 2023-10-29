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
