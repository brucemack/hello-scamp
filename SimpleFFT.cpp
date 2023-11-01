#include <complex>
#include <cstdlib>
#include <cmath>

#include "SimpleFFT.h"

using namespace std;

const float PI = 4.0f * atan(1.0f);
const float TWOPI = 2.0f * PI;

namespace scamp {

void simple_fft(complex<float> *input, complex<float> *output, int N, int step)
{
  if (step < N) 
  {
    // Recursive Functionality 
    simple_fft(output, input, N, step * 2);
    simple_fft(output + step, input + step, N, step * 2);
    
    // For loop for k cycles
    for (int k = 0; k < N; k += 2 * step)
    {
      float angle = -PI * k / N;
      float realPart, imagPart;      

      // Calculating the Complex Sum
      realPart = output[k + step].real() * cos(angle) - output[k + step].imag() * sin(angle);
      imagPart = output[k + step].real() * sin(angle) + output[k + step].imag() * cos(angle);

      complex<float> out(realPart, imagPart);

      input[k / 2] = output[k] + out;
      input[(k + N) / 2] = output[k] - out;
    }
  }
}

} // namespace