#include <iostream>
#include "FrameSenderImpl.h"
#include "scamp-util-1.h"
#include "scamp-util-2.h"
#include "Encoder.h"

using namespace std;

namespace scamp {

    static const uint32_t FRAME_MSB    = 0b100000000000000000000000000000;

    FrameSenderImpl::FrameSenderImpl() 
    :   _state(IDLE),
        _radio(0),
        _enabled(false),
        _radioEnabled(false),
        _sentFrameBits(0),
        _outQueueSize(0),
        _idleFrame(scamp::make_frame(kw4ti::golay_encode(0b000000000000))) {
    }

    void FrameSenderImpl::setRadio(Radio* radio) {
        _radio = radio;
    }

    bool FrameSenderImpl::isActive() {
        return _state != State::IDLE;
    }

    void FrameSenderImpl::bitTicks(unsigned int tickCount) {
        for (unsigned int i = 0; i < tickCount; i++) 
            bitTick();
    }

    void FrameSenderImpl::bitTick() {
        if (_state == State::IDLE) {
            if (_outQueueSize > 0) {
                _enabled = true;
                _state = State::WAITING_FOR_RADIO;
            }
        }
        else if (_state == State::WAITING_FOR_RADIO) {
            if (!_radio->isBusy()) {
                _radio->enable();
                _state = State::SENDING_SYNC0;
                _workingFrame = Encoder::SYNC_FRAME_0;
                _sentFrameBits = 0;
            }
        }
        else if (_state == State::SENDING_SYNC0 ||
                 _state == State::SENDING_SYNC1 ||
                 _state == State::SENDING_FRAME) {
            // If there are more bits in the frame, just keep sending them
            if (_sentFrameBits < 30) {
                _sendBit();
            } 
            // This is when we are finished with the frame
            else {
                // When SYNC0 is finished start to send SYNC1
                if (_state == State::SENDING_SYNC0) {
                    _state = State::SENDING_SYNC1;
                    _workingFrame = Encoder::SYNC_FRAME_1;
                    _sentFrameBits = 0;
                    _sendBit();
                }                
                // When SYNC1 or a normal data frame is finished start to send 
                // the next frame from the queue, or idle if there are none waiting.
                else if (_state == State::SENDING_SYNC1 || 
                         _state == State::SENDING_FRAME) {
                    // Anything pending?
                    if (_outQueueSize) {
                        _workingFrame = scamp::make_frame(kw4ti::golay_encode(_outQueue[0]));
                        _sentFrameBits = 0;
                        _sendBit();
                        // Shift down the out queue
                        for (unsigned int i = 0; i < _outQueueSize; i++)
                            _outQueue[i] = _outQueue[i + 1];
                        _outQueueSize--;
                    } 
                    // If there is nothing in the out queue then decide whether to idle
                    // or shut down
                    else {
                        if (_enabled) {
                            _workingFrame = _idleFrame;
                            _sentFrameBits = 0;
                            _sendBit();
                        } 
                        else {
                            // Shut down the radio and go into idle state
                            _radio->disable();
                            _radioEnabled = false;
                            _state = State::IDLE;
                        }
                    }
                }                
            }
        }
    }

    void FrameSenderImpl::disable() {
        _enabled = false;
    }

    void FrameSenderImpl::_sendBit() {
        // Do the actual modulation
        if (_workingFrame & FRAME_MSB) {
            _radio->sendMark();
        } else {
            _radio->sendSpace();
        }
        // Shift left 
        _workingFrame = _workingFrame << 1;
        _sentFrameBits++;
    }

    void FrameSenderImpl::queue(scampCodeWord12_t codeWord12) {
        if (_outQueueSize < FRAME_SENDER_QUEUE_SIZE) {
            _outQueue[_outQueueSize] = codeWord12;
            _outQueueSize++;
        }
    }

    bool FrameSenderImpl::canQueue() const {
        return _outQueueSize < FRAME_SENDER_QUEUE_SIZE;
    }
}
