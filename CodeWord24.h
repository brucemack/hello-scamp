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
#ifndef _CodeWord24_h
#define _CodeWord24_h

#include "CodeWord12.h"

namespace scamp {

    class CodeWord24 {
    public:

        static CodeWord24 fromCodeWord12(CodeWord12 cw);

        CodeWord24(uint32_t raw);

        CodeWord12 toCodeWord12() const;

        uint32_t getRaw() const;

    private:

        uint32_t _raw;
    };
}

#endif
