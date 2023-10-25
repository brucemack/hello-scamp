#ifndef _Symbol6_h
#define _Symbol6_h

#include <iostream>
#include <cstdint>

namespace scamp {

    class Symbol6 {
    public:

        static Symbol6 ZERO;
        static Symbol6 fromAscii(char asciiChar);

        Symbol6(uint8_t scamp6);

        uint8_t getRaw() const;

        char toAscii() const;

        /**
         * Used for creating the "reverse array" that maps ASCII characters
         * to SCAMP-6 characters.
         */
        static void writeReverse(std::ostream& str);

    private:

        uint8_t _raw;
    };
}

#endif
