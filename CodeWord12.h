#ifndef _CodeWord12_h
#define _CodeWord12_h

#include <cstdint>

#include "Symbol6.h"

namespace scamp {

    /**
     * A class used to represent the 12-bit SCAMP code word prior to the application
     * of the Golay encoding.  The code word typically contains two text symbols,
     * although there are other variations.
     */
    class CodeWord12 {
    public:

        static CodeWord12 fromSymbols(Symbol6 symbol0, Symbol6 symbol1);

        CodeWord12(uint16_t raw, bool valid);
        
        bool isValid() const;

        uint16_t getRaw() const;

        Symbol6 getSymbol0() const;
        Symbol6 getSymbol1() const;

    private:

        uint16_t _raw;
        bool _valid;
    };
}

#endif
