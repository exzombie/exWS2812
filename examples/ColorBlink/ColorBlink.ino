/*
  ColorBlink

  A short example on how to use the exWS2812 library.

  This example code is in the public domain.
*/

#include "exWS2812.h"

static const byte pixelsPin = 8;  // Where the LEDs are connected.

// The first parameter is the number of WS2812 LEDs, the other two represent the
// pin register. To obtain the correct register address, use the same pin number
// as above in the ObtainPinPortValues example sketch.
WS2812<3, 0x16, 2> threePixels;

void setup() {
  pinMode(pixelsPin, OUTPUT);  // Pin must be initialized manually.
  threePixels.begin();         // This prepares the pixels in "off" configuration.
}

void loop() {
  // Set the pixels to predefined colors.
  threePixels[0] = WS2812Color::red;
  threePixels[1] = WS2812Color::green;
  threePixels[2] = WS2812Color::blue;
  threePixels.commit();  // Transmits commands to the LEDs.
  delay(1000);

  // Set the pixels to cyan, magenta and yellow.
  threePixels[0] = 0x00FFFF;
  threePixels[1] = 0xFF00FF;
  threePixels[2] = 0xFFFF00;
  threePixels.commit();  // Transmits commands to the LEDs.
  delay(1000);

  // Keep the colors, but reduce their brightness.
  threePixels[0].dim(16);
  threePixels[1].dim(32);
  threePixels[2].dim(64);
  threePixels.commit();
  delay(1000);

  // Set each pixel to random color.
  for (byte i = 0; i < 3; ++i) {
    threePixels[i] = random(0xFFFFFF);
  }
  threePixels.commit();
  delay(1000);
}
