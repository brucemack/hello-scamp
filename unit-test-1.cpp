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

static void test_1() {
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
}

void make_tone(complex<float>* output, unsigned int len, float sample_freq_hz, 
  float tone_freq_hz, float amplitude) {
  float phi = 0;
  float omega = 2.0f * 3.1415926f * (tone_freq_hz / sample_freq_hz);
  for (unsigned int i = 0; i < len; i++) {
    output[i] = complex<float>(std::cos(phi) * amplitude);
    phi += omega;
  }
}

/** 
 * A single signal that mixes two tones.
 */
static void test_2() {

  const unsigned int N = 8;
  complex<float> inputs_0[N];
  complex<float> inputs_1[N];
  complex<float> inputs[N];
  complex<float> outputs[N];

  // Make two tones
  make_tone(inputs_0, N, 2000.0, 250.0, 0.5);
  make_tone(inputs_1, N, 2000.0, 500.0, 0.5);
  
  // Mix
  for (unsigned int i = 0; i < N; i++) {
    inputs[i] = inputs_0[i] + inputs_1[i];
    outputs[i] = inputs[i];
  }

  simple_fft(inputs, outputs, N);

  cout << setprecision(2);

  for (int i = 0; i < N; i++) {
      std::cout << std::abs(inputs[i]) << std::endl;
  }
}

/** 
 * A single signal that concatenates two tones.
 */
static void test_3() {

  const unsigned int N = 16;
  complex<float> inputs[N];
  complex<float> outputs[N];

  // Make two tones
  make_tone(inputs, N / 2, 2000.0, 500.0, 0.5);
  make_tone(inputs + N, N / 2, 2000.0, 250.0, 0.5);
  
  for (unsigned int i = 0; i < N; i++) {
    outputs[i] = inputs[i];
  }

  simple_fft(inputs, outputs, N);

  cout << setprecision(2);

  for (int i = 0; i < N; i++) {
      std::cout << std::abs(inputs[i]) << std::endl;
  }
}

int main(int argc, char *argv[]) {
  test_3();
}
