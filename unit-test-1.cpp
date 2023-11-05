#include <fstream>
#include <complex>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>

#include "SimpleFFT.h"
#include "Frame30.h"
#include "TestModem2.h"

using namespace std;
using namespace scamp;

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

const float PI = 3.1415926f;

ostream &operator<<(ostream &out, complex<float> c)
{
  if (c.imag() < 0) {
    out << c.real() << " - " << std::abs(c.imag()) << "j";
  } else {
    out << c.real() << " + " << c.imag() << "j";
  }
  return out;
}

void make_tone(complex<float>* output, unsigned int len, float sample_freq_hz, 
  float tone_freq_hz, float amplitude) {
  float phi = 0;
  float omega = 2.0f * PI * (tone_freq_hz / sample_freq_hz);
  for (unsigned int i = 0; i < len; i++) {
    output[i] = complex<float>(std::cos(phi) * amplitude);
    phi += omega;
  }
}

unsigned int max_mag_bin(complex<float>* output, unsigned int len) {
    float max = 0;
    unsigned int max_bin = 0;
    for (unsigned int i = 0; i < len; i++) {
        if (std::abs(output[i]) >= max) {
            max = std::abs(output[i]);
            max_bin = i;
        }
    }
    return max_bin;
}

static void test_1() {

    int N = 8;
    complex<float> buffer[8];
    complex<float> outputs[8];

    // Notice that this waveform will have a DC term 
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

// Real SCAMP signals
static void test_4() {

    float sampleFreq = 2000.0;
    //float toneFreq = 600.0;
    float toneFreq = 667.0;
    const unsigned int N = 64;
    complex<float> inputs[N];
    complex<float> outputs[N];

  make_tone(inputs, N, sampleFreq, toneFreq, 1.0);
  
  for (unsigned int i = 0; i < N; i++) {
    outputs[i] = inputs[i];
  }

    simple_fft(inputs, outputs, N);

    cout << setprecision(2);
    for (int i = 0; i < N / 2; i++) {
        float f = sampleFreq * (float)i / (float)N;
        std::cout << i << " " << f << ": " << std::abs(inputs[i]) << std::endl;
    }

    unsigned int m = max_mag_bin(inputs, N / 2);
    float max_f = sampleFreq * (float)m / (float)N;
    cout << "Max Bin " << m << " Freq " << max_f << " mag " << std::abs(inputs[m]) << endl;
}

// Real SCAMP signals
static void test_5() {

    // Make a few edges (6 bits)
    float sampleFreq = 2000.0;
    float markFreq = 667.0;
    float spaceFreq = 600.0;

    const unsigned int N = 64;
    complex<float> inputs[N * 6];

    float hann_window[N];
    for (unsigned int n = 0; n < N; n++) {
        hann_window[n] = std::pow(std::sin(PI * (float)n / (float)N), 2);
    }

    make_tone(inputs      , 60, sampleFreq, markFreq, 1.0);
    make_tone(inputs +  60, 60, sampleFreq, spaceFreq, 1.0);
    make_tone(inputs + 120, 60, sampleFreq, markFreq, 1.0);
    make_tone(inputs + 180, 60, sampleFreq, spaceFreq, 1.0);
    make_tone(inputs + 240, 60, sampleFreq, markFreq, 1.0);
    make_tone(inputs + 300, 60, sampleFreq, spaceFreq, 1.0);
    // Build out the complete 384 samples
    make_tone(inputs + 360, 24, sampleFreq, markFreq, 1.0);

    // Do a sliding window FFT
    for (unsigned w = 0; w < 384 - 16; w += 16) {

        // Make up some temporary structures for the FFT and apply 
        // the window.
        complex<float> window_inputs[N];
        complex<float> outputs[N];
        for (unsigned int i = 0; i < N; i++) {
            window_inputs[i] = inputs[w + i] * hann_window[i];
            outputs[i] = inputs[w + i];
        }
        simple_fft(window_inputs, outputs, N);

        // Analyze the result
        unsigned int m = max_mag_bin(window_inputs, N / 2);
        float max_f = sampleFreq * (float)m / (float)N;
        cout << w << ": Freq " << max_f << " mag " << std::abs(window_inputs[m]) << endl;
    }
}

// Real SCAMP signals
static void test_6() {

    unsigned int sampleFreq = 2000;
    unsigned int markFreq = 667;
    unsigned int spaceFreq = 600;
    const unsigned int samplesPerSymbol = 60;

    // Two complete frames of space
    const unsigned int S = 2 * 30 * samplesPerSymbol;
    float samples[S];

    TestModem2 modem(samples, S, sampleFreq, samplesPerSymbol, markFreq, spaceFreq);
    
    // Half a frame of silence
    for (unsigned int i = 0; i < 15; i++) {
        modem.sendSilence();
    }
    // Start frame
    Frame30::START_FRAME.transmit(modem);
    // Half a frame of silence
    for (unsigned int i = 0; i < 15; i++) {
        modem.sendSilence();
    }

    assertm(modem.getSamplesUsed() == S, "Modem sample count");

    // for (unsigned int i = 0; i < S; i++) {
    //     cout << i << " " << samples[i] << endl;
    // }

    // Try to find the tone
    const unsigned int N = 64;
    complex<float> window_inputs[N];
    complex<float> outputs[N];

    float hannWindow[N];
    for (unsigned int n = 0; n < N; n++) {
        hannWindow[n] = std::pow(std::sin(PI * (float)n / (float)N), 2);
    }

    // Do a sliding window FFT
    for (unsigned w = 0; w < S - N; w += 16) {

        for (unsigned int i = 0; i < N; i++) {
            window_inputs[i] = samples[w + i];
            outputs[i] = window_inputs[i];
        }
        simple_fft(window_inputs, outputs, N);

        // Analyze the result
        unsigned int m = max_mag_bin(window_inputs, N / 2);
        float max_f = sampleFreq * (float)m / (float)N;
        cout << w << ": Freq " << max_f << " mag " << std::abs(window_inputs[m]) << endl;
    }
}

int main(int argc, char *argv[]) {
  test_1();
}
