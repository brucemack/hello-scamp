#ifndef _FrameReceiver_h
#define _FrameReceiver_h

#include <cinttypes>
#include "Util.h"
#include "Decoder.h"

namespace scamp {

    class FrameReceiver {
    public:

        FrameReceiver();

        void setDecoder(Decoder* decoder);

        void bitTick(bool mark);

    private:

        void _processFrame(scampFrame30_t rawFrame);

        uint32_t _accumulator;
        bool _frameSync;
        unsigned int _bitCount;
    };
}
#endif