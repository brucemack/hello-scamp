#ifndef _Encoder_h
#define _Encoder_h

#include <cinttypes>

namespace scamp {

    class FrameSender {
    public:

        virtual bool canQueue() const = 0;
        virtual void queue(uint16_t codeWord12) = 0;
    };

    class Encoder {
    public:

        Encoder();

        void setFrameSender(FrameSender* sender);

        void sendString(const char* asciiString);

        const static uint32_t SYNC_FRAME_0;
        const static uint32_t SYNC_FRAME_1;
        const static uint32_t FRAME_MASK;
        const static uint8_t SCAMP6_TO_ASCII8[];

    private:

        uint8_t _ascii8ToScamp6(uint8_t asciiChar) const;
        void _sendChar(uint8_t asciiChar);        

        FrameSender* _sender;
        unsigned int _count;
        uint8_t _hold;
    };
}

#endif

