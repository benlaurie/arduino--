#ifndef SPI_H_
#define SPI_H

#include "arduino--.h"

template <class Sck, class Miso, class Mosi, class Ss> 
    class _SPI
    {
public:

    static void wait()
        {
        while(!(SPSR & (1 << SPIF)))
            ;
        }
    
    // Default is MSB first, SPI mode 0, FOSC/4
    static void init(byte config = 0, bool double_speed = false)
        {
        // initialize the SPI pins
        Sck::modeOutput();
        Mosi::modeOutput();
        Miso::modeInput();
        Ss::modeOutput();
        Ss::set();
        
        mode(config, double_speed);
        }
    
    static void mode(byte config, bool double_speed)
        {
        byte tmp __attribute__((unused));

        // enable SPI master with configuration byte specified
        Register::SPCR = 0;
        Register::SPCR = (config & 0x7F) | (1 << SPE) | (1 << MSTR);
        // clear any pending conditions and set double speed if needed.
        if (double_speed)
            SPSR |= (1 << SPI2X);
        else
            tmp = SPSR;
        tmp = SPDR;
        }
    
    static byte transfer(byte value, byte delay = 0)
        {
        Ss::clear();
        SPDR = value;
        wait();
        Ss::set();
        if (delay > 0)
            ::delayMicroseconds(delay);
        return SPDR;
        }
    };

class NullPin
    {
public:
    static void modeOutput() { }
    static void modeInput() { }
    static void set() { }
    static void clear() { }
    // Using read() or toggle() will trigger a compilation error.
    };


typedef _SPI<Pin::SPI_SCK, Pin::SPI_MISO, Pin::SPI_MOSI, NullPin> SPI;
typedef _SPI<Pin::SPI_SCK, Pin::SPI_MISO, Pin::SPI_MOSI, Pin::SPI_SS> SPISS;

#endif
