/*
SCAMP Encoder/Decoder
Copyright (C) 2023 - Bruce MacKinnon KC1FSZ

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _Modulator_h
#define _Modulator_h

namespace scamp {

    /**
     * A generic interface for a modulator that would be driven by an encoder.
     */
    class Modulator {
    public:

        virtual ~Modulator() { }

        virtual void sendSilence(uint32_t durationUs) { }
        virtual void sendMark(uint32_t durationUs) = 0;
        virtual void sendSpace(uint32_t durationUs) = 0;
    };
}

#endif 
