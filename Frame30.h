#ifndef _Frame30_h
#define _Frame30_h

#include "CodeWord24.h"
#include "Modulator.h"

namespace scamp {

    class Frame30 {
    public:

        static Frame30 ZERO_FRAME;
        // Start of transmission frames, contains edges to synchronize with
        static Frame30 SYNC_FRAME_0;
        static Frame30 SYNC_FRAME_1;

        static Frame30 fromCodeWord24(CodeWord24 cw);

        /** 
         * Correlates the first 30 LSB bits of the 32 bit number.
         */
        static int32_t correlate30(uint32_t a, uint32_t b);

        Frame30(uint32_t raw);

        uint32_t getRaw() const;

        bool isValid() const;

        CodeWord24 toCodeWord24() const;

        /**
         * Returns the number of compliment sets.  Expected value is 6.
         */
        unsigned int getComplimentCount() const;

        /**
         * Transmits frame contents
         */
        void transmit(Modulator& modulator);

    private:

        uint32_t _raw;
    };
}

#endif
