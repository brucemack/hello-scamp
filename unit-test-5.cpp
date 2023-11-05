#include <iostream>

extern "C" {
    #include "fixmath.h"
}

using namespace std;

int main(int argc, const char** argv) {
    int16_t a = 4;
    int16_t b = 4;
    int16_t m = fix16_mul(a, b);
    cout << m << endl;
}

