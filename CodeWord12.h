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
*/
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
