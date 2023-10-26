#include <iostream>
#include <cmath>
#include "ClockRecoveryPLL.h"

using namespace std;
using namespace scamp;

int main(int argc, const char** argv) {

    ClockRecoveryPLL pll(2000);

    int data[] = { 0,0,0,0,0,0,0,0,
                   1,1,1,1,1,1,1,1,
                   0,0,0,0,0,0,0,0,
                   1,1,1,1,1,1,1,1,
                   0,0,0,0,0,0,0,0 };

    for (int k = 0; k < 100; k++) {
        for (int i = 0; i < sizeof(data); i++) {
            bool mark = data[i] ? true : false;
            pll.processSample(mark);
            cout << "INT " << pll.getErrorIntegration() << "   PER " << pll.getDCOPeriod() << endl;
        }
    }

    return 0;
}
