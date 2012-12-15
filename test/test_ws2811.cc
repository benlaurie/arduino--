#include "arduino--.h"

typedef Pin::D4 Zero;  // convenient so we have GND on the next pin
typedef Pin::D5 LEDS;

class LEDController
    {
public:
    static void One() __attribute__((always_inline))
	{
	// High for 1.2 us, low for 1.3 us
	LEDS::set();
	__builtin_avr_delay_cycles(17);
 	//_delay_us(1.2 - .175 + .025 - .0375);
	LEDS::clear();
	//_delay_us(1.3 - .8125);
	}

    static void Zero() __attribute__((always_inline))
	{
	LEDS::set();
	_delay_us(.5 - .125);
	LEDS::clear();
	_delay_us(2.0 - .75 - .625);
	}

    static void Reset()
	{
	LEDS::clear();
	_delay_us(50);
	}
    };

class FastLEDController
    {
public:
    static void One()
	{
	// High for .6 us, low for .65 us
	LEDS::set();
	_delay_us(1.2 - .175 + .025 - .2375);
	LEDS::clear();
	//	_delay_us(1.3);
	// external code takes 1.4375 us!
	}

    static void Zero()
	{
	LEDS::set();
	_delay_us(.5 - .125);
	LEDS::clear();
	//_delay_us(2.0 - 1.25 - .625);
	}

    static void Reset()
	{
	LEDS::clear();
	_delay_us(50);
	}
    };

int main(void)
    {
    //    static byte base[] = { "\xff\xff\x00\x00Mary had a little lamb\xa5\xa5\x5a\x5a" };
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
    byte buf[sizeof(base) - 1];
    Nanode::init();

    Zero::modeOutput();
    Zero::clear();
    LEDS::modeOutput();
    for (uint16_t offset = 0 ; ; offset += 3)
	{
	for (uint16_t n = 0; n < sizeof buf; ++n)
	    buf[n] = base[(n + offset) % sizeof buf];
	if (offset == 3000)
	    offset = 0;

	byte *pb = buf;
	LEDController::Reset();
	for (uint16_t n = 0; n < sizeof buf; ++n, ++pb)
	    {
	    byte b = *pb;
	    for (byte bit = 0; bit < 8; ++bit)
		{
		if (bit)
		    __builtin_avr_delay_cycles(8);
		if (b & 1)
		    LEDController::One();
		else
		    LEDController::Zero();
		b >>= 1;

		}
	    }
	_delay_ms(100);
	}
    }
