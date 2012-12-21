#include "arduino--.h"

typedef Pin::D4 Zero;  // convenient so we have GND on the next pin
typedef Pin::D5 LEDS;

#define NOP 	__asm__("\tnop\n")

// This has not been tested and timings are rather tight, but it might work.
class FastLEDController
    {
public:
    static void One() __attribute__((always_inline))
	{
	// High for .6 us, low for .65 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastOne(byte **pb) __attribute__((always_inline))
	{
	// High for .6 us, low for .65 us
	LEDS::set();
	++*pb;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	}

    static void Zero() __attribute__((always_inline))
	{
	// High for .25 us, low for 1 us
	LEDS::set();
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastZero(byte **pb) __attribute__((always_inline))
	{
	// High for .25 us, low for 1 us
	LEDS::set();
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	++*pb;
	}

    static void Reset()
	{
	LEDS::clear();
	_delay_us(50);
	}
    };

class LEDController
    {
public:
    static void One() __attribute__((always_inline))
	{
	// High for 1.2 us, low for 1.3 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastOne(byte **pb) __attribute__((always_inline))
	{
	// High for 1.2 us, low for 1.3 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	++*pb;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void Zero() __attribute__((always_inline))
	{
	// High for .5 us, low for 2 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastZero(byte **pb) __attribute__((always_inline))
	{
	// High for .5 us, low for 2 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	++*pb;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void Reset()
	{
	LEDS::clear();
	_delay_us(50);
	}
    };

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
#include "test.grb"
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

	byte *pb = buf;
	byte *pe = buf + sizeof buf;
	LEDController::Reset();
	for (; pb < pe; )
	    {
	    byte b = *pb;
#define F(x) \
	    if (b & x) \
		LEDController::One(); \
	    else \
		LEDController::Zero()
	    F(0x80);
	    F(0x40);
	    F(0x20);
	    F(0x10);
	    F(0x08);
	    F(0x04);
	    F(0x02);
	    //F(0x01); ++pb;
	    if (b & 0x01)
		LEDController::LastOne(&pb);
	    else
		LEDController::LastZero(&pb);
	    }
	}
    }
