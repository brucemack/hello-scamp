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
#include <complex>
#include <cstdlib>
#include <cmath>

#include "SimpleFFT.h"

using namespace std;

const float PI = 4.0f * atan(1.0f);
const float TWOPI = 2.0f * PI;

namespace scamp {

static void simple_fft_recursive(complex<float> *input, complex<float> *output, int N, int step) {
  if (step < N) {

    simple_fft_recursive(output, input, N, step * 2);
    simple_fft_recursive(output + step, input + step, N, step * 2);
    
    for (int k = 0; k < N; k += 2 * step) {

      float angle = -PI * k / N;      
      float realPart = output[k + step].real() * cos(angle) - output[k + step].imag() * sin(angle);
      float imagPart = output[k + step].real() * sin(angle) + output[k + step].imag() * cos(angle);

      complex<float> out(realPart, imagPart);

      input[k / 2] = output[k] + out;
      input[(k + N) / 2] = output[k] - out;
    }
  }
}

void simple_fft(complex<float>* input, complex<float>* output, int N) {
    simple_fft_recursive(input, output, N, 1);
}


} // namespace