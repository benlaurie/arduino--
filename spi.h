#ifndef SPI_H_
#define SPI_H

#include "arduino++.h"

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
    static void init(byte config = 0)
        {
        // initialize the SPI pins
        Sck::modeOutput();
        Mosi::modeOutput();
        Miso::modeInput();
        Ss::modeOutput();
        Ss::set();
        
        mode(config);
        }
    
    static void mode(byte config)
        {
        byte tmp;
        
        // enable SPI master with configuration byte specified
        SPCR = 0;
        SPCR = (config & 0x7F) | (1 << SPE) | (1 << MSTR);
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

#endif
