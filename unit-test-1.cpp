#include <iostream>
#include <cassert>

#include "scamp-util-1.h"
#include "scamp-util-2.h"
#include "FrameSenderImpl.h"
#include "Encoder.h"
#include "FrameReceiver.h"

using namespace std;

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

// ====== DUMMY RADIO =======
class MockRadio : public scamp::Radio {
public:

    MockRadio();

    void setFrameReceiver(scamp::FrameReceiver* rec);

    virtual bool isBusy() const;
    virtual void sendMark();
    virtual void sendSpace();
    virtual void enable();
    virtual void disable();

private:

    scamp::FrameReceiver* _recv;
};

MockRadio::MockRadio() 
:   _recv(0) {
}

void MockRadio::setFrameReceiver(scamp::FrameReceiver* rec) {
    _recv = rec;
}

bool MockRadio::isBusy() const {
    return false;
}

void MockRadio::sendMark() {

    cout << "1";

    if (_recv) {
        _recv->bitTick(true);
    }
}

void MockRadio::sendSpace() {

    cout << "0";

    if (_recv) {
        _recv->bitTick(false);
    }
}

void MockRadio::enable() {
    cout << endl;
    cout << "(Enabled)" << endl;
}

void MockRadio::disable() {
    cout << endl;
    cout << "(Disabled)" << endl;
}

int main(int argc, const char** argv) {

    // Make sure the code is reversible
    {
        // Test 12-bit codeword
        const scampCodeWord12_t wd = 0b100100100100;
        // Coded 
        const uint16_t wd_enc = kw4ti::golay_mult(wd);
        // Reverse
        const uint16_t wd1 = kw4ti::golay_mult(wd_enc);
        // Sanity check
        assertm(wd == wd1, "Reverse test failure");
    }

    // Demonstrate a code/decode sequence with an error injected
    {
        const uint8_t data_x = 0b010000;
        const uint8_t data_y = 0b111110;
        // Make the code word
        const scampCodeWord12_t data_12 = (data_y << 6) | data_x;
        const scampCodeWord24_t code_word_24 = kw4ti::golay_encode(data_12);

        // Inject an error 
        const scampCodeWord24_t damaged_code_word_24 = code_word_24 ^ 0b000100000100;

        // Decode
        uint8_t biterrors = 0;
        const scampCodeWord12_t recovered_data_12 = kw4ti::golay_decode(damaged_code_word_24, &biterrors);

        // Check 
        assertm(data_12 == recovered_data_12, "Error correct problem");
        assertm(biterrors == 2, "Wrong number of bad bits");
    }

    // Make a frame 
    {
        const scampCodeWord24_t code_word_24 = 0b000000000000000000000000;
        const scampFrame30_t expected_frame_30 = 0b100001000010000100001000010000;
        const scampFrame30_t frame_30 = scamp::make_frame(code_word_24);
        assert(expected_frame_30 == frame_30);
    }

    {
        const uint8_t data_x = 0b010000;
        const uint8_t data_y = 0b111110;
        const scampCodeWord12_t data_12 = (data_y << 6) | data_x;
        const scampCodeWord24_t code_word_24 = kw4ti::golay_encode(data_12);
    }

    // Correlation with synchronization frame
    {
        scampFrame30_t w0 = scamp::Encoder::SYNC_FRAME_1;
        scampFrame30_t w1 = scamp::Encoder::SYNC_FRAME_1;
        assert(scamp::correlate(w0, w1) == 30);
        assert(scamp::correlate(w0, ~w1) == -30);
        // Mess up one of the bits
        assert(abs(scamp::correlate(w0, w1 ^ 0b0001000)) >= 28);
        // Shift to show that there is no auto-correlation
        assert(abs(scamp::correlate(w0, w1 >> 1)) < 5);
        assert(abs(scamp::correlate(w0, w1 << 1)) < 5);
    }

    // Frame sending test
    {
        MockRadio radio;
        scamp::FrameSenderImpl sender;
        sender.setRadio(&radio);
        assert(sender.canQueue());
        sender.queue(0);
        // This will move from idle
        sender.bitTick();
        // This will turn on the radio (if available)
        sender.bitTick();
        // Shut down (once the data is out)
        sender.disable();
        // We should see the sync frames and then one data frame
        sender.bitTicks(30);
        sender.bitTicks(30);
        // Here we see the zero frame sent
        sender.bitTicks(30);
        // Here we see the radio turned off
        sender.bitTick();
    }

    // Encoder test
    {
        MockRadio radio;
        scamp::FrameSenderImpl sender;
        sender.setRadio(&radio);
        scamp::Encoder encoder;
        encoder.setFrameSender(&sender);
        encoder.sendString("AB");

        // This will move from idle
        sender.bitTick();
        // This will turn on the radio (if available)
        sender.bitTick();
        // Shut down (once the data is out)
        sender.disable();
        // We should see the sync frames and then one data frame
        sender.bitTicks(30);
        sender.bitTicks(30);
        // Here we see the AB frame sent
        sender.bitTicks(30);
        // Here we see the radio turned off
        sender.bitTick();
    }

    // Encoder/Decoder test
    {
        cout << "----- Decoder Test -----" << endl;

        MockRadio radio;
        scamp::FrameSenderImpl sender;
        sender.setRadio(&radio);
        scamp::Encoder encoder;
        encoder.setFrameSender(&sender);

        scamp::FrameReceiver recv;
        radio.setFrameReceiver(&recv);

        encoder.sendString("ABC");

        // This will move from idle
        sender.bitTick();
        // This will turn on the radio (if available)
        sender.bitTick();
        // Shut down (once the data is out)
        sender.disable();
        // We should see the sync frames and then one data frame
        sender.bitTicks(30);
        sender.bitTicks(30);
        // Here we see the AB frame sent
        sender.bitTicks(30);
        // Here we see the C_ frame sent
        sender.bitTicks(30);
        // Here we see the radio turned off
        sender.bitTick();
    }


    return 0;
}
