
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>
#include <sstream>
#include <string>
#include <iomanip>    

#include "Util.h"
#include "Symbol6.h"
#include "CodeWord24.h"
#include "Frame30.h"
#include "FileModulator.h"
#include "TestModem.h"
#include "ClockRecoveryPLL.h"

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

    // =========================================================================
    // Make a message and modulate it into a file. This demonstration is used
    // for creating .WAV files.
    {
        std::ofstream outfile("./scamp-fsk-slow-demo-0.txt");
        // This is SCAMP FSK SLOW
        FileModulator mod(outfile, 2000, 144, 667, 625);

        for (unsigned int i = 0; i < 30; i++)
            mod.sendSilence();

        // Send the synchronization frame
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

        for (unsigned int i = 0; i < 30; i++)
            mod.sendSilence();

        outfile.close();
    }

    // =========================================================================
    // Encoding tests
    {        
        std::function<void(char a, char b)> f2 = [](char a, char b) {  
        };
        makePairs("abc", f2);

        // Validate assignment
        Frame30 a(0xaa);
        Frame30 b = a;
        assertm(b.getRaw() == 0xaa, "Default assignment");
    }

    // =========================================================================
    // Make a message, encode it, and then recover it. This test assumes one 
    // sample per symbol so there is no bit clock synchronization going on here. 
    // We are just testing the ability to recognize the synchronization frame.
    {        
        const char* testMessage = "DE KC1FSZ, GOOD MORNING";

        // Make enough room to capture the tones
        int8_t samples[1024];
        TestModem modem(samples, sizeof(samples), 1);

        {
            // Make the message
            Frame30 frames[32];
            unsigned int count = encodeString(testMessage, frames, 32, true);
            assertm(count == 14, "Frame count problem");

            // Send some silence and other garbage at the beginning
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendMark();
            modem.sendMark();
            modem.sendSpace();
            modem.sendMark();
            modem.sendSilence();
            modem.sendSilence();

            // Transmit the legit message
            for (unsigned int i = 0; i < count; i++) {
                frames[i].transmit(modem);
            }
        }

        // Decode the message
        ostringstream outStream;
        {
            uint32_t accumulator = 0;
            bool inSync = false;
            unsigned int bitCount = 0;

            for (unsigned int i = 0; i < modem.getSamplesUsed(); i++) {

                // Bring in the next bit
                accumulator <<= 1;
                if (samples[i] == 1) {
                    accumulator |= 1;
                }
                bitCount++;
                
                if (!inSync) {
                    // Look for sync frame
                    if (Frame30::correlate30(accumulator, Frame30::SYNC_FRAME_1.getRaw()) > 29) {
                        inSync = true;
                        bitCount = 0;
                    }
                }
                // Here we are consuming real frames
                else {
                    if (bitCount == 30) {
                        bitCount = 0;
                        Frame30 frame(accumulator & Frame30::MASK30LSB);
                        if (!frame.isValid()) {                                            
                            cout << endl << "WARNING: Invalid frame" << endl;
                        } 
                        CodeWord24 cw24 = frame.toCodeWord24();
                        CodeWord12 cw12 = cw24.toCodeWord12();
                        Symbol6 sym0 = cw12.getSymbol0();
                        Symbol6 sym1 = cw12.getSymbol1();
                        if (sym0.getRaw() != 0) {
                            outStream << sym0.toAscii();
                        }
                        if (sym1.getRaw() != 0) {
                            outStream << sym1.toAscii();
                        }
                    }
                }
            }
        }

        // Validate the recovered message
        assertm(outStream.str() == testMessage, "Message error");
    }

    // =========================================================================
    // Make a message, encode it, and then recover it. In this test we are 
    // using the SCAMP FSK (33.3 bits per second) format.  The clock recovery
    // PLL is used to enable synchronization of the bit stream.
    {        
        const char* testMessage = "DE KC1FSZ, GOOD MORNING";

        // Make enough room to capture the tones
        int8_t samples[60 * 1024];
        // The 60 means 60 samples per symbol - consistent with 33.3 symbols/second
        TestModem modem(samples, sizeof(samples), 60);

        // Encode the message.  This leads to about 25K samples.
        {
            Frame30 frames[32];
            unsigned int count = encodeString(testMessage, frames, 32, true);

            // Send some silence (not a full frame)
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();
            modem.sendSilence();

            // Transmit a legit message
            for (unsigned int i = 0; i < count; i++) {
                frames[i].transmit(modem);
            }
        }

        // Now decode
        ostringstream outStream;
        {
            ClockRecoveryPLL pll(2000);
            // Purposely set for the wrong frequency to watch the clock 
            // recovery work. But it's close.
            pll.setBitFrequencyHint(36);
           
            uint32_t accumulator = 0;
            bool inSync = false;
            unsigned int bitCount = 0;
            ios init(NULL);
            init.copyfmt(cout);

            for (unsigned int i = 0; i < modem.getSamplesUsed(); i++) {

                if (i % 60 == 0) {
                    cout << endl;
                    cout << setw(4) << (i / 60) << " [";
                    cout << setw(6) << pll.getLastError() << "]: ";
                    cout.copyfmt(init);
                }

                // Feed the clock recovery PLL
                bool captureSample = pll.processSample(samples[i] == 1);
                cout << (samples[i] == 1);
                // We only process a bit when the PLL tells us to
                if (!captureSample) {
                    continue;
                }

                cout << "|";
                //cout << "Bit sync " << i << ", error " << pll.getLastError() << endl;

                // Bring in the next bit
                accumulator <<= 1;
                if (samples[i] == 1) {
                    accumulator |= 1;
                }
                bitCount++;
                
                if (!inSync) {
                    // Look for sync frame
                    if (Frame30::correlate30(accumulator, Frame30::SYNC_FRAME_1.getRaw()) > 29) {
                        inSync = true;
                        bitCount = 0;
                        //cout << "Sync frame" << endl;
                    }
                }
                // Here we are consuming real frames
                else {
                    if (bitCount == 30) {
                        bitCount = 0;
                        Frame30 frame(accumulator & Frame30::MASK30LSB);
                        if (!frame.isValid()) {                                            
                            cout << endl << "WARNING: Invalid frame" << endl;
                        } 
                        CodeWord24 cw24 = frame.toCodeWord24();
                        CodeWord12 cw12 = cw24.toCodeWord12();
                        Symbol6 sym0 = cw12.getSymbol0();
                        Symbol6 sym1 = cw12.getSymbol1();
                        if (sym0.getRaw() != 0) {
                            outStream << sym0.toAscii();
                        }
                        if (sym1.getRaw() != 0) {
                            outStream << sym1.toAscii();
                        }
                    }
                }
            }
        }
        cout.flush();

        // Validate the recovered message
        assertm(outStream.str() == testMessage, "Message error");
    }
}
