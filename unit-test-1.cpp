#include <fstream>
#include <complex>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>

using namespace std;

const double PI = 4.0f * atan(1.0f);
const double TWOPI = 2.0f * PI;

ostream &operator<<(ostream &out, complex<float> c)
{
  if (c.imag() < 0) {
    out << c.real() << " - " << std::abs(c.imag()) << "j";
  } else {
    out << c.real() << " + " << c.imag() << "j";
  }
  return out;
}

void fft(complex<float> *input, complex<float> *output, int N, int step)
{
  if (step < N) 
  {
    // Recursive Functionality 
    fft(output, input, N, step * 2);
    fft(output + step, input + step, N, step * 2);
    
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

int main(int argc, char *argv[]) 
{
    int N = 8;
    complex<float> buffer[8];
    complex<float> outputs[8];

    buffer[0] = complex<float>(1, 0);
    buffer[1] = complex<float>(0, 0);
    buffer[2] = complex<float>(1, 0);
    buffer[3] = complex<float>(0, 0);
    buffer[4] = complex<float>(1, 0);
    buffer[5] = complex<float>(0, 0);
    buffer[6] = complex<float>(1, 0);
    buffer[7] = complex<float>(0, 0);

    cout << setprecision(4);

    for (int i = 0; i < N; i++)
    {
        outputs[i] = buffer[i];
    }

    // Calculate FFT
    fft(buffer, outputs, N, 1);

    // Print out the output
    for (int i = 0; i < N; i++)
    {
        std::cout << buffer[i] << "  " << std::abs(buffer[i]) << std::endl;
    }

    return 0;
}
