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
