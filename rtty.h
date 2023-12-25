#ifndef _rtty_h
#define _rtty_h

#include <cstdint>
#include "DataListener.h"

/*
From: https://www.aa5au.com/rtty/diddles-by-w7ay/

Even though Baudot is a 5 bit character code, 3 extra bits are added 
to provide character synchronization.  A start bit (space tone) is 
prepended to the Baudot code, and two stop bits (mark tone) are 
added after the Baudot code.  Thus the actual character is 8 bits.

In “rest” or idle condition, RTTY sends a continuous Mark tone.  After 
an idle (Mark) period, the Baudot stream decoder is going to wait 
for the first space tone to be demodulated.  The Baudot decoder then 
assumes this is a Start bit.  It then assumes that the tone 1/45.45 
seconds later represents the first bit of the Baudot character, the 
one 2/45.45 seconds later represents the second bit of the Baudot 
character, and so on.
*/

class BaudotDecoder {
public:

    BaudotDecoder(uint16_t sampleRate, uint16_t baudRateTimes100);

    void setDataListener(DataListener* l) { _listener = l; };

    void reset();

    /**
     * Symbol 1 = Mark (High)
     * Symbol 0 = Space (Low)
    */
    void processSymbol(uint8_t symbol);

private:    

    uint16_t _sampleRate;
    uint16_t _baudRateTimes100;
    bool _inFigs;

    DataListener* _listener;
};

#endif
