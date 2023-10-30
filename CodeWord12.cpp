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
#include "CodeWord12.h"

namespace scamp {

CodeWord12 CodeWord12::fromSymbols(Symbol6 s0, Symbol6 s1) {
    uint16_t r = (s0.getRaw() & 0b111111) | ((s1.getRaw() & 0b111111) << 6);
    return CodeWord12(r, true);
}

CodeWord12::CodeWord12(uint16_t raw, bool valid) 
:   _raw(raw),
    _valid(valid) { }
        

bool CodeWord12::isValid() const {
    return _valid;
}

uint16_t CodeWord12::getRaw() const {
    return _raw;
}

Symbol6 CodeWord12::getSymbol0() const {
    return Symbol6(_raw & 0b111111);
}

Symbol6 CodeWord12::getSymbol1() const {
    return Symbol6((_raw >> 6) & 0b111111);
}

}
