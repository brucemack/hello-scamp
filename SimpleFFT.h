#ifndef _SimpleFFT_h
#define _SimpleFFT_h

#include <complex>

namespace scamp {

void simple_fft(std::complex<float> *input, std::complex<float> *output, int N, int step);

}

#endif

