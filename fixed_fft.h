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

PRIOR ART
=========
This borrows heavily from code by Hunter Adams (vha3@cornell.edu), 
Tom Roberts, and Malcolm Slaney (malcolm@interval.com).
*/
#ifndef _fixed_fft_h
#define _fixed_fft_h

#include <iostream>
#include "fixed_math.h"

/**
 * A Q15 fixed-point FFT implementation using the original Danielson-Lanczos 
 * algorithm.
 * 
 * The goal is to create a straight-forward implementation that can 
 * be built on a desktop or embedded platforms.
 */
template<unsigned int N> class FixedFFT {
public:

    FixedFFT() {
        for (unsigned int i = 0; i < N; i++) {
            _cosTable[i] = float_to_q15(std::sin(TWO_PI * ((float) i) / (float)N));
        }
    }

    /**
     * Performs the FFT in-place. Meaning: the input series is overwritten.
     */
    void transform(q15 fr[], q15 fi[]) const {

        // One of the indices being swapped    
        uint16_t m;   
        // The other index being swapped (r for reversed)
        uint16_t mr; 
        // Temporary while swapping
        q15 tr, ti; 

        // Indices being combined in Danielson-Lanczos part of the algorithm    
        int16_t i, j; 
        // Used for looking up trig values
        int16_t k;    
        
        // Length of the FFT which results from combining two FFT's
        int16_t istep; 
        
        // -----------------------------------------------------------------------
        // The bit-reversal phase of the algorithm, based on this:
        // https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious

        for (m = 1; m < N - 1; m++) {
            // Swap odd and even bits
            mr = ((m >> 1) & 0x5555) | ((m & 0x5555) << 1);
            // Swap consecutive pairs
            mr = ((mr >> 2) & 0x3333) | ((mr & 0x3333) << 2);
            // Swap nibbles 
            mr = ((mr >> 4) & 0x0F0F) | ((mr & 0x0F0F) << 4);
            // Swap bytes
            mr = ((mr >> 8) & 0x00FF) | ((mr & 0x00FF) << 8);
            // Shift down mr
            mr >>= _shiftAmount;
            // Don't swap that which has already been swapped
            if (mr <= m) continue;
            // Swap the bit-reversed indices
            tr = fr[m];
            fr[m] = fr[mr];
            fr[mr] = tr;
            ti = fi[m];
            fi[m] = fi[mr];
            fi[mr] = ti;
        }

        // -----------------------------------------------------------------------
        // The Danielson-Lanczos algorithm adapted from code by:
        // Tom Roberts 11/8/89 and Malcolm Slaney 12/15/94 malcolm@interval.com

        // Length of the FFT's being combined (starts at 1)
        int16_t L = 1;
        // Log2 of number of samples, minus 1
        k = _log2N - 1;
        // While the length of the FFTs being combined is less than the number 
        // of gathered samples:
        while (L < N) {
            // Determine the length of the FFT which will result from combining two FFT's
            istep = L << 1;
            // For each element in the FFT's that are being combined
            for (m = 0; m < L; ++m) { 
                // Lookup the trig values for that element
                j = m << k;                
                // cos(2PI m/N)
                q15 wr = _cosTable[j + N / 4]; 
                // sin(2PI m/N)
                q15 wi = -_cosTable[j];                 
                // Divide by two
                wr >>= 1;                          
                // Divide by two
                wi >>= 1;
                // i gets the index of one of the FFT elements being combined
                for (i = m; i < N; i += istep) {
                    // j gets the index of the FFT element being combined with i
                    j = i + L;
                    // Compute the trig terms (bottom half of the above matrix)
                    tr = mult_q15(wr, fr[j]) - mult_q15(wi, fi[j]);
                    ti = mult_q15(wr, fi[j]) + mult_q15(wi, fr[j]);
                    // Divide ith index elements by two (top half of above matrix)
                    q15 qr = fr[i] >> 1;
                    q15 qi = fi[i] >> 1;
                    // Compute the new values at each index
                    fr[j] = qr - tr;
                    fi[j] = qi - ti;
                    fr[i] = qr + tr;
                    fi[i] = qi + ti;
                }    
            }
            --k;
            L = istep;
        }
    }

    uint16_t binToFreq(uint16_t bin, uint16_t sampleFreq) {
        return (bin * sampleFreq) / N;
    }

private: 

    const uint16_t _log2N = std::log2(N);
    const uint16_t _shiftAmount = 16 - _log2N;
    const float PI = std::atan(1.0) * 4.0;
    const float TWO_PI = PI * 2.0f;

    // Pre-built table for fast trig
    q15 _cosTable[N]; 
};

#endif
