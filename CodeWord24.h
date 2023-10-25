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
