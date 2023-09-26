#ifndef _FrameSender
#define _FrameSender

#include "FrameSender.h"

#define FRAME_SENDER_QUEUE_SIZE 64

namespace scamp {

    class Radio {
    public:

        virtual bool isBusy() const = 0;
        virtual void sendMark() = 0;
        virtual void sendSpace() = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
    };

    class FrameSenderImpl : public FrameSender {
    public:

        enum State {
            IDLE,
            WAITING_FOR_RADIO,
            SENDING_SYNC0,
            SENDING_SYNC1,
            SENDING_FRAME
        };

        FrameSenderImpl();

        void setRadio(Radio* radio);

        void bitTick();
        void bitTicks(unsigned int tickCount);
        void disable();
        bool canQueue() const;
        void queue(scampCodeWord12_t codeWord12);
        bool isActive();
    
    private:

        void _sendBit();

        State _state;
        Radio* _radio;
        bool _enabled;
        bool _radioEnabled;
        scampFrame30_t _workingFrame;
        unsigned int _sentFrameBits;
        // This queue contains 12-bit code words (i.e. they have not been Golay encoded yet)
        scampCodeWord12_t _outQueue[FRAME_SENDER_QUEUE_SIZE];
        unsigned int _outQueueSize;
        const scampFrame30_t _idleFrame;
    };
}

#endif
