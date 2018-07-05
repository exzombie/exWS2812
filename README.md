# exWS2812

Driver code for WS2812B RGB LED strips for 8 Mhz and 16 Mhz AVR
microcontrollers. Implemented as a templated class, making code both
readable (no `ifdef`s) and fast, allowing use of any port and
pin. Also included is a Color class which will instantiate a color
from a hex value or a list of bytes. Look at the included example and
the documentation in the header itself.

The only gotcha is that you need to know the port and bit of the pin
you are using. If you are not using Arduino, you already know where to
get that. If you are using Arduino, the information is available in
the Arduino framework, but is not easy to get at in the form needed
here. There is a sketch included in the examples which prints the
Arduino pin numbers and the corresponding port and bit numbers to use
with the WS2812 template.

## Authors

Written by Jure Varlec <jure@varlec.si>, published under the GPL3 licence.
