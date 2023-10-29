#include "Util.h"
#include "Symbol6.h"

namespace scamp {

void makePairs(const char* in, std::function<void(char a, char b)> cb) {
    const char* p = in;
    while (*p != 0) {
        char a = *p;
        p++;
        char b;
        if (*p == 0) {
            b = 0;
        } else {
            b = *p;
            p++;
        }
        cb(a, b);
    }
}

unsigned int encodeString(const char* in, Frame30* outList, unsigned int outListSize, 
    bool includeSyncFrame) {

    unsigned int used = 0;

    if (includeSyncFrame) {
        if (used < outListSize) {
            outList[used++] = Frame30::SYNC_FRAME_0;
        }
        if (used < outListSize) {
            outList[used++] = Frame30::SYNC_FRAME_1;
        }
    }

    // Make a Lambda that takes the next two characters in the input and 
    // adds a frame to the output list.
    makePairs(in, 
        //std::function<void(char a, char b)> procPair = 
        [&used, outList, outListSize](char a, char b) {  
            if (used < outListSize) {
                outList[used++] = Frame30::fromTwoAsciiChars(a, b);
            }
        }
    );
    // Trigger the frame building
    //makePairs(in, procPair);
    return used;
}

} // namespace

