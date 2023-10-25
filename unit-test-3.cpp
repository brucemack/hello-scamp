
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>

#include "Symbol6.h"
#include "CodeWord24.h"
#include "Frame30.h"
#include "FileModulator.h"

using namespace std;
using namespace scamp;

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

// Test
class TestModulator : public Modulator {
public:

    void sendMark() { cout << "1"; }
    void sendSpace() { cout << "0"; }
};

int main(int argc, const char** argv) {

    // Make sure the code is reversible
    {
        // Test 12-bit codeword
        CodeWord12 wd12(0b100100100100, true);
        // Coded 
        CodeWord24 wd24 = CodeWord24::fromCodeWord12(wd12);
        // Reverse
        CodeWord12 wd12_b = wd24.toCodeWord12();
        // Sanity check
        assertm(wd12.getRaw() == wd12_b.getRaw(), "Reverse test failure");
    }

    // Demonstrate a code/decode sequence with an error injected
    {
        const uint8_t data_x = 0b010000;
        const uint8_t data_y = 0b111110;
        // Make the code word
        const uint16_t data_12 = (data_y << 6) | data_x;
        CodeWord12 wd12(data_12, true);
        // Coded 
        CodeWord24 wd24 = CodeWord24::fromCodeWord12(wd12);
        // Inject an error into the raw message
        const uint32_t damaged_code_word_24 = wd24.getRaw() ^ 0b000100000100;
        CodeWord24 wd24_b(damaged_code_word_24);

        // Decode
        CodeWord12 wd12_b = wd24_b.toCodeWord12();

        // Check 
        assertm(wd12.getRaw() == wd12_b.getRaw(), "Error correct problem");
        assertm(wd12_b.isValid(), "Validity check problem");
    }

    // Demonstrate non-recoverable
    {
        const uint8_t data_x = 0b010000;
        const uint8_t data_y = 0b111110;
        // Make the code word
        const uint16_t data_12 = (data_y << 6) | data_x;
        CodeWord12 wd12(data_12, true);
        // Coded 
        CodeWord24 wd24 = CodeWord24::fromCodeWord12(wd12);
        // Inject an error into the raw message
        const uint32_t damaged_code_word_24 = wd24.getRaw() ^ 0b000110100100;
        CodeWord24 wd24_b(damaged_code_word_24);

        // Decode
        CodeWord12 wd12_b = wd24_b.toCodeWord12();

        // Check 
        assertm(!wd12_b.isValid(), "Validity check problem");
    }

    // Make a frame 
    {
        CodeWord24 cw(0b000000000000000000000000);
        Frame30 frame = Frame30::fromCodeWord24(cw);
        const uint32_t expected_frame = 0b100001000010000100001000010000;
        assertm(expected_frame == frame.getRaw(), "Frame problem");
    }

    // Make a 12-bit code word from two ASCII letters
    {
        Symbol6 s0 = Symbol6::fromAscii('D');
        assertm(s0.getRaw() == 0x21, "ASCII conversion problem");
        Symbol6 s1 = Symbol6::fromAscii('E');
        assertm(s1.getRaw() == 0x22, "ASCII conversion problem");
        CodeWord12 cw = CodeWord12::fromSymbols(s0, s1);
        assertm(cw.getRaw() == 0x8A1, "Text codeword formation problem");
    }

    // Make a frame
    {
        Symbol6 s0 = Symbol6::fromAscii('D');
        Symbol6 s1 = Symbol6::fromAscii('E');
        CodeWord12 cw12 = CodeWord12::fromSymbols(s0, s1);
        CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
        Frame30 frame = Frame30::fromCodeWord24(cw24);
        // Make sure the frame is valid
        assertm(6 == frame.getComplimentCount(), "Frame compliment problem");
        /*
        cout << std::hex << "0x" << frame.getRaw() << endl;
        TestModulator mod;
        frame.transmit(mod);
        cout << endl;
        */
    }

    /*
    // Test file
    {
        std::ofstream outfile("./scamp-fsk-slow-demo-0.txt");
        FileModulator mod(outfile, 2000, 144, 667, 625);

        for (unsigned int i = 0; i < 20; i++) {
            mod.sendMark();
            mod.sendSpace();
        }

        outfile.close();
    }
    */

    // Make a message and modulate it
    {
        std::ofstream outfile("./scamp-fsk-slow-demo-0.txt");
        // This is SCAMP FSK SLOW
        FileModulator mod(outfile, 2000, 144, 667, 625);

        // Send the synchronization frame
        Frame30::ZERO_FRAME.transmit(mod);

        Frame30::SYNC_FRAME_0.transmit(mod);
        Frame30::SYNC_FRAME_1.transmit(mod);

        {
            Symbol6 s0 = Symbol6::fromAscii('D');
            Symbol6 s1 = Symbol6::fromAscii('E');
            CodeWord12 cw12 = CodeWord12::fromSymbols(s0, s1);
            CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
            Frame30 frame = Frame30::fromCodeWord24(cw24);
            frame.transmit(mod);
        }
        {
            Symbol6 s0 = Symbol6::fromAscii(' ');
            Symbol6 s1 = Symbol6::fromAscii('K');
            CodeWord12 cw12 = CodeWord12::fromSymbols(s0, s1);
            CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
            Frame30 frame = Frame30::fromCodeWord24(cw24);
            frame.transmit(mod);
        }
        {
            Symbol6 s0 = Symbol6::fromAscii('C');
            Symbol6 s1 = Symbol6::fromAscii('1');
            CodeWord12 cw12 = CodeWord12::fromSymbols(s0, s1);
            CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
            Frame30 frame = Frame30::fromCodeWord24(cw24);
            frame.transmit(mod);
        }
        {
            Symbol6 s0 = Symbol6::fromAscii('F');
            Symbol6 s1 = Symbol6::fromAscii('S');
            CodeWord12 cw12 = CodeWord12::fromSymbols(s0, s1);
            CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
            Frame30 frame = Frame30::fromCodeWord24(cw24);
            frame.transmit(mod);
        }
        {
            Symbol6 s0 = Symbol6::fromAscii('Z');
            Symbol6 s1 = Symbol6::ZERO;
            CodeWord12 cw12 = CodeWord12::fromSymbols(s0, s1);
            CodeWord24 cw24 = CodeWord24::fromCodeWord12(cw12);
            Frame30 frame = Frame30::fromCodeWord24(cw24);
            frame.transmit(mod);
        }

        Frame30::ZERO_FRAME.transmit(mod);

        outfile.close();
    }
}
