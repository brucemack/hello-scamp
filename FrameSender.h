#ifndef _FrameSender_h
#define _FrameSender_h

#include <cinttypes>
#include "Util.h"

namespace scamp {

    class FrameSender {
    public:

        virtual bool canQueue() const = 0;
        virtual void queue(scampCodeWord12_t cw) = 0;
    };

}

#endif
