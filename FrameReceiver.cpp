#include <iostream>
#include "FrameReceiver.h"
#include "scamp-util-1.h"
#include "scamp-util-2.h"
#include "Encoder.h"

using namespace std;

namespace scamp {

FrameReceiver::FrameReceiver() 
{
}

void FrameReceiver::setDecoder(Decoder* decoder) {
}

void FrameReceiver::bitTick(bool mark) {

    _accumulator <<= 1;

    if (mark) {
        _accumulator |= 1;
    }

    _bitCount++;

    // Test the correlation to see if we've found a frame sync
    int32_t c = scamp::correlate(_accumulator, Encoder::SYNC_FRAME_1);
    if (c == 30) {
        //cout << "[SYNC FOUND]";
        _frameSync = true;
        _bitCount = 0;
    }  

    if (_frameSync) {
        if (_bitCount == 30) {
            _processFrame(Encoder::FRAME_MASK & _accumulator);
            _bitCount = 0;
        }
    }
}

void FrameReceiver::_processFrame(scampFrame30_t frame) {

    // Undo the Golay encoding
    uint8_t bitErrors;
    scampCodeWord12_t codeWord = kw4ti::golay_decode(scamp::frame_to_codeword24(frame), 
        &bitErrors);

    uint8_t xxxxxx = codeWord & 0b111111;
    uint8_t yyyyyy = (codeWord >> 6) & 0b111111;

    if (xxxxxx) {
        char asc = Encoder::SCAMP6_TO_ASCII8[xxxxxx];
        cout << "[" << asc << "]";
    }

    if (yyyyyy) {
        char asc = Encoder::SCAMP6_TO_ASCII8[yyyyyy];
        cout << "[" << asc << "]";
    }
}

}
