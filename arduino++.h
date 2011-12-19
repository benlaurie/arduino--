#include <avr/io.h>
#include "avr-ports.h"

typedef uint8_t byte;

class Arduino
    {
public:
    static void WaitSPI()
	{
	while(!(SPSR & (1 << SPIF)))
	    ;
	}
    };

template <byte ddr, byte port, byte in, byte bit> class _Pin
    {
public:
    static void Out() { _SFR_IO8(ddr) |= _BV(bit); }
    static void In() { _SFR_IO8(ddr) &= ~_BV(bit); }
    static void Set() { _SFR_IO8(port) |= _BV(bit); }
    static void Clear() { _SFR_IO8(port) &= ~_BV(bit); }
    static byte Read() { return !!(_SFR_IO8(in) & _BV(bit)); }
    };

class Pin
    {
public:
    typedef _Pin<NDDRB, NPORTB, NPINB, PB2> B2;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB3> B3;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB4> B4;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB5> B5;

    typedef _Pin<NDDRC, NPORTC, NPINC, PC4> C4;
    typedef _Pin<NDDRC, NPORTC, NPINC, PC3> C3;

    typedef B3 SPI_MOSI;
    typedef B4 SPI_MISO;
    typedef B5 SPI_SCK;
    };

