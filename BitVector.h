#ifndef _BitVector_h
#define _BitVector_h

namespace scamp {

    class BitVector {
    public:

        virtual unsigned int size() = 0;
        virtual bool get(unsigned int i) = 0;
    };

}

#endif
