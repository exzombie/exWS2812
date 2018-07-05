#pragma once

#include <util/atomic.h>
#include <avr/cpufunc.h>
#include "initializer_list.h"

#ifdef ARDUINO

struct WS2812PortAndPin {
    char port = -1;
    char bit = -1;

    WS2812PortAndPin(byte pinNumber) {
        char portId = digitalPinToPort(pinNumber);
        if (portId == NOT_A_PIN) {
            return;
        }
        char theport = portInputRegister(portId) - __SFR_OFFSET;
        byte bitmask = digitalPinToBitMask(pinNumber);
        char i;
        for (i = 0; !(bitmask & _BV(i)) && i < 8; ++i);
        if (i == 8) {
            return;
        }
        port = theport;
        bit = i;
    }
};

#endif

/*
 * WS2812Color represents a color in RGB space. The three bytes for R, G and B
 * are in RGB order in the interface, but are stored in GRB order for efficient
 * transmission to a WS2812 LED. There are several ways to construct a color:
 *
 *     WS2812Color pink(255, 192, 203);
 *     WS2812Color pink = {255, 192, 203};
 *     WS2812Color pink = 0xFFC0CB;
 *     WS2812Color dimPink(16, 255, 192, 203);
 *     WS2812Color dimPink = {16, 255, 192, 203};
 *     WS2812Color dimPink = 0x0FFFC0CB;
 *     WS2812Color rnd = random();  // Works on Arduino, where random() returns a long.
 *
 * Brightness is specified by giving four values, the first being the
 * brightness. The lowest brightness is 1; the value zero is the same as 255.
 * Brightness of an existing color can be reduced by calling mycolor.dim(). A
 * new, dimmer color can be created from an existng color by calling
 * mycolor.dimmer(). This is useful for the fixed basic colors that are
 * provided, such as WS2812Color::red. Example:
 *
 *     WS2812Color dimPink1 = pink;
 *     dimPink1.dim(16);
 *     WS2812Color dimPink2 = pink.dimmer(16);
 *     WS2812Color dimRed = WS2812Color::red.dimmer(32);
 */

struct WS2812Color {
    union {
        struct { byte g; byte r; byte b; };
        byte bytes[3] = {0, 0, 0};
    };

    WS2812Color() = default;

    WS2812Color(byte _r, byte _g, byte _b) {
        g = _g;
        r = _r;
        b = _b;
    }

    WS2812Color(byte brightness, byte _r, byte _g, byte _b) {
        g = _g;
        r = _r;
        b = _b;
        dim(brightness);
    }

    WS2812Color(std::initializer_list<byte> list) {
        if (list.size() == 0) return;
        auto rgb = list.end() - 1;
        auto start = list.begin() - 1;
        b = *rgb;
        if (--rgb == start) return;
        g = *rgb;
        if (--rgb == start) return;
        r = *rgb;
        if (--rgb == start) return;
        if (*rgb != 0) {
            dim(*rgb);
        }
    }

    WS2812Color(uint32_t rgb) {
        byte* bt = reinterpret_cast<byte*>(&rgb);
        // Little endian.
        g = bt[1];
        r = bt[2];
        b = bt[0];
        if (bt[3] != 0) {
            dim(bt[3]);
        }
    }

    void dim(byte brightness) {
        for (byte i = 0; i < 3; ++i) {
            bytes[i] = static_cast<uint16_t>(bytes[i]) * brightness / 256;
        }
    }

    WS2812Color dimmer(byte brightness) const {
        WS2812Color c = *this;
        c.dim(brightness);
        return c;
    }

    static const WS2812Color& off;
    static const WS2812Color black;
    static const WS2812Color red;
    static const WS2812Color green;
    static const WS2812Color blue;
    static const WS2812Color white;
};

const WS2812Color& WS2812Color::off = black;
const WS2812Color WS2812Color::black = 0;
const WS2812Color WS2812Color::red = 0xff0000;
const WS2812Color WS2812Color::green = 0x00ff00;
const WS2812Color WS2812Color::blue = 0x0000ff;
const WS2812Color WS2812Color::white = 0xffffff;


/*
 * WS2812 is a template class that handles storage and transmission of color
 * data for each LED. It requires one pin, 8MHz or 16Mhz CPU clock and suport
 * for the FastPin library. Instatiate the class with the number of WS2812 LEDs
 * and the pin number, then use begin() to enable the pin. Use setColor() to
 * pick a color for each LED and commit() to submit data to the LEDs. For
 * example, to pick random colors once per second with a strip of 5 LEDs
 * connected to pin 10:
 *
 *     WS2812<5, 10> ledStrip;
 *
 *     void setup() {
 *         ledStrip.begin();
 *     }
 *
 *     void loop() {
 *         for (byte l = 0; l < ledStrip.size(); ++l) {
 *             ledStrip.setColor(l, random());
 *         }
 *         ledStrip.commit(); delay(1000);
 *     }
 *
 *  There are also getColor() and operator []. You can call setColor() and
 *  getColor() with no LED number and they will get/set the first LED; useful
 *  if you only have one LED.
 */

template <byte numLeds, byte pinPort, byte pinBit>
class WS2812 {

static_assert(F_CPU == 8000000 || F_CPU == 16000000,
              "Unsupported CPU clock.");

static_assert(numLeds * sizeof(WS2812Color) < 256, "Too many LEDs.");

public:
    using Color = WS2812Color;

    void begin() {
        for (byte i = 0; i < numBytes; ++i) {
            dataBytes[i] = 0;
        }
    }

    byte size() const {
        return numLeds;
    }

    void setColor(byte led, const Color& color) {
        data[led] = color;
    }

    void setColor(const Color& color) {
        setColor(0, color);
    }

    const Color& getColor(byte led) const {
        return data[led];
    }

    const Color& getColor() const {
        return data[0];
    }

    Color& operator[](byte led) {
        return data[led];
    }

    const Color& operator[](byte led) const {
        return data[led];
    }

    void commit() const;

private:
    Color data[numLeds];
    byte* dataBytes = reinterpret_cast<byte*>(data);
    static const byte numBytes = numLeds * sizeof(Color);

    inline void togglePin() const __attribute__((always_inline));
};



template <byte numLeds, byte pinPort, byte pinBit>
void WS2812<numLeds, pinPort, pinBit>::togglePin() const {
    asm volatile("sbi %[port], %[bit] \n\t"
        :: [port] "I" (pinPort), [bit] "I" (pinBit));
}

template <byte numLeds, byte pinPort, byte pinBit>
void WS2812<numLeds, pinPort, pinBit>::commit() const {
ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (byte N = 0; N < numBytes; ++N) {
        byte b = dataBytes[N];
        for (byte i = 0; i < 8; ++i, b <<= 1) {
            if (b & _BV(7)) {
                // Send a 1
                if (F_CPU == 16000000) {
                    togglePin();
                    _NOP(); _NOP(); _NOP();
                    _NOP(); _NOP(); _NOP();
                    _NOP(); _NOP(); _NOP();
                    togglePin();
                } else if (F_CPU == 8000000) {
                    togglePin();
                    _NOP(); _NOP(); _NOP(); _NOP(); _NOP();
                    togglePin();
                }
            } else {
                // Send a 0
                if (F_CPU == 16000000) {
                    togglePin();
                    _NOP(); _NOP(); _NOP();
                    togglePin();
                    _NOP(); _NOP(); _NOP(); _NOP(); _NOP();
                } else if (F_CPU == 8000000) {
                    togglePin();
                    _NOP();
                    togglePin();
                    _NOP(); _NOP(); _NOP(); _NOP();
                }
            }
        };
    }
}
}
