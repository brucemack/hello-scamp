#include <iostream>

#define FIXED_POINT 16

extern "C" {
    #include "kissfft/kiss_fft.h"
}

using namespace std;

kiss_fft_scalar mag(kiss_fft_cpx cx) {
    return std::sqrt((cx.r * cx.r) + (cx.i * cx.i));
}

int main(int argc, const char** argv) {

    unsigned int N = 8;
    // Space for the state (272 bytes + 8 * (N-1))
    unsigned int state_area[34 + 7];
    size_t state_area_size = 328;
    kiss_fft_cfg cfg = kiss_fft_alloc( N, 0, (void*)state_area, &state_area_size);
    cout << "Size " << state_area_size << endl;

    kiss_fft_cpx cx_in[8];
    kiss_fft_cpx cx_out[8];

    // Load the input
    cx_in[0].r = 1; cx_in[0].i = 0;
    cx_in[1].r = 0; cx_in[1].i = 0;
    cx_in[2].r = 1; cx_in[2].i = 0;
    cx_in[3].r = 0; cx_in[3].i = 0;
    cx_in[4].r = 1; cx_in[4].i = 0;
    cx_in[5].r = 0; cx_in[5].i = 0;
    cx_in[6].r = 1; cx_in[6].i = 0;
    cx_in[7].r = 0; cx_in[7].i = 0;

    kiss_fft( cfg, cx_in , cx_out );

    // Display result
    for (int i = 0; i < 8; i++) {
        cout << i << " " << cx_out[i].r << endl;
    }

    return 0;   
}
