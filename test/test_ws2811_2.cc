#include "arduino--.h"
#include "ws2811.h"

typedef Pin::D4 Zero;  // convenient so we have GND on the next pin
typedef Pin::D5 LEDS;  // used for initialisation

DEFINE_WS2811_FN(WS2811RGB, PORTD, 5)


int main(void)
    {
    //static byte buf[] = { "\xff\xff\x00\x00Mary had a little lamb\xa5\xa5\x5a\x5a" };
    // Note that the strip I have, at least, is GRB instead of RGB.
    /*
    static byte base[] = { "\xff\xff\xff"
			   "\xff\xff\x00"
			   "\xff\x00\x00"
			   "\x00\xff\x00"
			   "\x00\x00\xff"
			   "\x00\xff\xff"
			   "\xff\x00\xff"
			   "\x77\x77\x77"
			   "\x00\x77\x77"
			   "\x00\x00\x77"
			   "\x00\x77\x00"
			   "\x77\x00\x77"
			   "\x77\x77\x00"
			   "\x00\x00\x00"
    };
    */
    static byte base[] = {
#include "test.rgb"
    };
    byte buf[sizeof(base)];
    Nanode::init();

    Zero::modeOutput();
    Zero::clear();
    LEDS::modeOutput();
    for (uint16_t offset = 0 ; ; offset += 3)
	{
	for (uint16_t n = 0; n < sizeof buf; ++n)
	    buf[n] = base[(n + offset) % sizeof buf];
	if (offset >= sizeof buf)
	    offset = 0;

	_delay_us(50);
	WS2811RGB((RGB_t *)buf, sizeof buf/3);
	}
    }
