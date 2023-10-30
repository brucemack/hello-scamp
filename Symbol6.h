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
