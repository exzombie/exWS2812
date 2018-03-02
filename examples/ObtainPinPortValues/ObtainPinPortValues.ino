/*
  ObtainPinPortValues

  This sketch prints Arduino pin numbers and their corresponding register
  addresses and bit numbers. These can then be used with the exWS2812 library.

  This example code is in the public domain.
*/

#include "exWS2812.h"

void setup() {
  Serial.begin(9600);
}

void loop() {
  WS2812PortAndPin addrs(0);
  Serial.println(F("Pin\tPort\tBit"));
  for (byte pinNum = 0; addrs.port != -1; addrs = WS2812PortAndPin(++pinNum)) {
    Serial.print(pinNum);
    Serial.print("\t0x");
    Serial.print(String((int)addrs.port, HEX));
    Serial.print("\t");
    Serial.print((int)addrs.bit);
    Serial.println();
  }
  delay(3000);
  Serial.println();
}
