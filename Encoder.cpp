#include "Encoder.h"
#include <stdio.h>

namespace scamp {

const uint32_t Encoder::SYNC_FRAME_0 = 0b111111111111111111111111010101;
const uint32_t Encoder::SYNC_FRAME_1 = 0b111110110100011001110100011110;
const uint32_t Encoder::FRAME_MASK   = 0b111111111111111111111111111111;

const uint8_t Encoder::SCAMP6_TO_ASCII8[] = {
   0x00, '\b', '\r',  ' ',  '!', '\"', '\'',  '(',
    ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',
    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
    '9',  ':',  ';',  '=',  '?',  '@',  'A',  'B',
    'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',
    'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',
    'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
   '\\',  '^',  '`',  '~', 0xff, 0xff, 0xff, 0xff
};

Encoder::Encoder() 
:  _sender(0),
   _count(0) {
}

void Encoder::setFrameSender(FrameSender* sender) {
   _sender = sender;
}

uint8_t Encoder::_ascii8ToScamp6(uint8_t asciiChar) const {
   if (asciiChar == 0) {
      return 0;
   }
   // Scan the table to find a match
   for (uint8_t i = 0; i < sizeof(SCAMP6_TO_ASCII8); i++) {
      if (SCAMP6_TO_ASCII8[i] == asciiChar) {
         return i;          
      }
   }
   // Default is ?
   return 20;
}

void Encoder::_sendChar(uint8_t asciiChar) {   
   // We transmit in pairs so the even characters get held for the next cycle
   if (_count % 2 == 0) {
      _hold = asciiChar;
   } 
   else {
      // When an odd character is received we make a frame
      uint16_t xxxxxxx = _ascii8ToScamp6(_hold);
      uint16_t yyyyyyy = _ascii8ToScamp6(asciiChar);
      uint16_t codeWord12 = (yyyyyyy << 6) | xxxxxxx;
      printf("DEBUG %X\n", codeWord12);
      _sender->queue(codeWord12);
   }
   _count++;
}

void Encoder::sendString(const char* sendStr) {
   
   uint8_t lastChar = 0;
   unsigned int count = 0;

   for (const uint8_t* s = (const uint8_t*)sendStr; *s != 0; s++) {
      _sendChar(*s);
      count++;
      // Look for repeats and send an extra
      if (s != (const uint8_t*)sendStr && lastChar == *s) {
         _sendChar(0);
         count++;
      }
      lastChar = *s;
   }

   // Make sure we are sending an even number of characters
   if (count % 2 == 1) {
      _sendChar(0);
   }
}

}