#include <cmath>

#include "fixed_fft.h"

// Macros for Q15 math
#define absq15(a) abs(a) 
#define int2q15(a) ((q15)(a << 15))
#define fix2int15(a) ((int)(a >> 15))
#define char2q15(a) (q15)(((q15)(a)) << 15)

// Number of samples per FFT
//#define NUM_SAMPLES 1024
//#define NUM_SAMPLES 512
#define NUM_SAMPLES 256
// Number of samples per FFT, minus 1
//#define NUM_SAMPLES_M_1 1023
//#define NUM_SAMPLES_M_1 511
#define NUM_SAMPLES_M_1 255
// Length of short (16 bits) minus log2 number of samples (10)
//#define SHIFT_AMOUNT 6
//#define SHIFT_AMOUNT 7
#define SHIFT_AMOUNT 8
// Log2 number of samples
//#define LOG2_NUM_SAMPLES 10
//#define LOG2_NUM_SAMPLES 9
#define LOG2_NUM_SAMPLES 8

// Sine table for the FFT calculation
static q15 Sinewave[NUM_SAMPLES]; 

const float PI = 3.1415926f;
const float TWO_PI = PI * 2.0f;

void fixed_fft_setup() {
    for (unsigned int i = 0; i < NUM_SAMPLES; i++) {
        Sinewave[i] = float2q15(std::sin(TWO_PI * ((float) i) / (float)NUM_SAMPLES));
        //HannWindow[i] = float2q15(0.5 * (1.0 - std::cos(TWO_PI * ((float) i) / ((float)NUM_SAMPLES))));
    }
}

// Peforms an in-place FFT. For more information about how this
// algorithm works, please see https://vanhunteradams.com/FFT/FFT.html
void fixed_fft(q15 fr[], q15 fi[]) {
    
    unsigned short m;   // one of the indices being swapped
    unsigned short mr ; // the other index being swapped (r for reversed)
    q15 tr, ti ; // for temporary storage while swapping, and during iteration
    
    int i, j ; // indices being combined in Danielson-Lanczos part of the algorithm
    int L ;    // length of the FFT's being combined
    int k ;    // used for looking up trig values from sine table
    
    int istep ; // length of the FFT which results from combining two FFT's
    
    q15 wr, wi ; // trigonometric values from lookup table
    q15 qr, qi ; // temporary variables used during DL part of the algorithm
    
    //////////////////////////////////////////////////////////////////////////
    ////////////////////////// BIT REVERSAL //////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Bit reversal code below based on that found here: 
    // https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
    for (m=1; m<NUM_SAMPLES_M_1; m++) {
        // swap odd and even bits
        mr = ((m >> 1) & 0x5555) | ((m & 0x5555) << 1);
        // swap consecutive pairs
        mr = ((mr >> 2) & 0x3333) | ((mr & 0x3333) << 2);
        // swap nibbles ... 
        mr = ((mr >> 4) & 0x0F0F) | ((mr & 0x0F0F) << 4);
        // swap bytes
        mr = ((mr >> 8) & 0x00FF) | ((mr & 0x00FF) << 8);
        // shift down mr
        mr >>= SHIFT_AMOUNT ;
        // don't swap that which has already been swapped
        if (mr<=m) continue ;
        // swap the bit-reveresed indices
        tr = fr[m] ;
        fr[m] = fr[mr] ;
        fr[mr] = tr ;
        ti = fi[m] ;
        fi[m] = fi[mr] ;
        fi[mr] = ti ;
    }
    //////////////////////////////////////////////////////////////////////////
    ////////////////////////// Danielson-Lanczos //////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Adapted from code by:
    // Tom Roberts 11/8/89 and Malcolm Slaney 12/15/94 malcolm@interval.com
    // Length of the FFT's being combined (starts at 1)
    L = 1 ;
    // Log2 of number of samples, minus 1
    k = LOG2_NUM_SAMPLES - 1 ;
    // While the length of the FFT's being combined is less than the number 
    // of gathered samples . . .
    while (L < NUM_SAMPLES) {
        // Determine the length of the FFT which will result from combining two FFT's
        istep = L<<1 ;
        // For each element in the FFT's that are being combined . . .
        for (m=0; m<L; ++m) { 
            // Lookup the trig values for that element
            j = m << k ;                         // index of the sine table
            wr =  Sinewave[j + NUM_SAMPLES/4] ; // cos(2pi m/N)
            wi = -Sinewave[j] ;                 // sin(2pi m/N)
            wr >>= 1 ;                          // divide by two
            wi >>= 1 ;                          // divide by two
            // i gets the index of one of the FFT elements being combined
            for (i=m; i<NUM_SAMPLES; i+=istep) {
                // j gets the index of the FFT element being combined with i
                j = i + L ;
                // compute the trig terms (bottom half of the above matrix)
                tr = multq15(wr, fr[j]) - multq15(wi, fi[j]) ;
                ti = multq15(wr, fi[j]) + multq15(wi, fr[j]) ;
                // divide ith index elements by two (top half of above matrix)
                qr = fr[i]>>1 ;
                qi = fi[i]>>1 ;
                // compute the new values at each index
                fr[j] = qr - tr ;
                fi[j] = qi - ti ;
                fr[i] = qr + tr ;
                fi[i] = qi + ti ;
            }    
        }
        --k ;
        L = istep ;
    }
}
