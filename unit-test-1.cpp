#include <fstream>
#include <complex>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>

#include "SimpleFFT.h"

using namespace std;
using namespace scamp;

ostream &operator<<(ostream &out, complex<float> c)
{
  if (c.imag() < 0) {
    out << c.real() << " - " << std::abs(c.imag()) << "j";
  } else {
    out << c.real() << " + " << c.imag() << "j";
  }
  return out;
}

int main(int argc, char *argv[]) {

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

    for (int i = 0; i < N; i++) {
        outputs[i] = buffer[i];
    }

    simple_fft(buffer, outputs, N);

    for (int i = 0; i < N; i++) {
        std::cout << buffer[i] << "  " << std::abs(buffer[i]) << std::endl;
    }

    return 0;
}
