#include <iostream>
#include <cmath>

using namespace std;

int main(int argc, const char** argv) {
    // Make a sine wave
    unsigned int sample_hz = 2000;
    unsigned int freq = 700;
    unsigned int dur = 1;
    const float PI = 3.1415926;
    float phase_per_sample = (2.0 * PI * (float)freq) / (float)sample_hz;
    cout << "Phase " << phase_per_sample << endl;

    float phi = 0;
    float block[128];

    const int m = 8;
    const int N = 24;

    const float I8[m] = { 16384, -11585, 0, 11585, -16384, 11585, 0, -11585 };
    const float Q8[m] = { 0, 11585, -16384, 11585, 0, -11585, 16384, -11585 };
    float x[N], yi[N], yq[N];

    for (unsigned int i = 0; i < N; i++) {
        x[i] = 0;
        yi[i] = 0;
        yq[i] = 0;
    }

    unsigned int n = 0;
    unsigned int n_minus_1 = N - 1;
    
    for (unsigned int i = 0; i < 256; i++) {
        float a = cos(phi);
        phi += phase_per_sample;
        float sample = a * 8192.0;
        // Grab the sample before overwriting it
        float old_x = x[n];
        x[n] = sample;
        // FIR
        yi[n] = yi[n_minus_1] + I8[n % m] * (x[n] - old_x);
        yq[n] = yq[n_minus_1] + Q8[n % m] * (x[n] - old_x);
        
        float mag = max(abs(yi[n]), abs(yq[n])) + floor((abs(yi[n]) + abs(yq[n])) / 2);
        cout << (int)(mag / 1000000.0) << endl;
        
        n_minus_1 = n;
        n = (n + 1) % N;
    }
}