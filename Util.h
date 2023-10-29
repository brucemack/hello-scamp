#ifndef _Util_h
#define _Util_h

#include <functional>
#include <cinttypes>
#include "Frame30.h"

namespace scamp {

/**
 * Takes a string and returns it in char pairs.
*/
void makePairs(const char* in, std::function<void(char a, char b)> cb);

/**
 * @returns The number of frames written into the list
*/
unsigned int encodeString(const char* in, Frame30* outList, unsigned int outListSize, 
    bool includeSyncFrame);

}

#endif
