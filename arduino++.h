#ifndef ARDUINO_MINUS_MINUS
#define ARDUINO_MINUS_MINUS

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr-ports.h"

typedef uint8_t byte;

template <byte ddr, byte port, byte in, byte bit, byte pcport, 
  byte pcbit, byte pcen> class _Pin
    {
public:
    static void enablePCInterrupt() { 
    PCICR |= _BV(pcen);
    _SFR_IO8(pcport) |= _BV(pcbit); 
    }
    static void disablePCInterrupt() { 
    if (_SFR_IO8(pcport) &= ~_BV(pcbit))
        PCICR &= ~_BV(pcen);
        
    }

    static void modeOutput() { _SFR_IO8(ddr) |= _BV(bit); }
    static void modeInput() { _SFR_IO8(ddr) &= ~_BV(bit); }
    static void set() { _SFR_IO8(port) |= _BV(bit); }
    static void clear() { _SFR_IO8(port) &= ~_BV(bit); }

    /** Return true if the Pin reads HIGH */
    static byte read() { return !!(_SFR_IO8(in) & _BV(bit)); }
    static byte toggle() { return (_SFR_IO8(port) ^= _BV(bit)); }
    };

class Pin
    {
public:
    typedef _Pin<NDDRB, NPORTB, NPINB, PB0, NPCMSK0, PCINT0, PCIE0> B0;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB1, NPCMSK0, PCINT1, PCIE0> B1;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB2, NPCMSK0, PCINT2, PCIE0> B2;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB3, NPCMSK0, PCINT3, PCIE0> B3;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB4, NPCMSK0, PCINT4, PCIE0> B4;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB5, NPCMSK0, PCINT5, PCIE0> B5;
        
    typedef _Pin<NDDRC, NPORTC, NPINC, PC0, NPCMSK1, PCINT8, PCIE1> C0;
    typedef _Pin<NDDRC, NPORTC, NPINC, PC1, NPCMSK1, PCINT9, PCIE1> C1;
    typedef _Pin<NDDRC, NPORTC, NPINC, PC2, NPCMSK1, PCINT10, PCIE1> C2;
    typedef _Pin<NDDRC, NPORTC, NPINC, PC3, NPCMSK1, PCINT11, PCIE1> C3;
    typedef _Pin<NDDRC, NPORTC, NPINC, PC4, NPCMSK1, PCINT12, PCIE1> C4;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD0, NPCMSK2, PCINT16, PCIE2> D0;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD1, NPCMSK2, PCINT17, PCIE2> D1;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD2, NPCMSK2, PCINT18, PCIE2> D2;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD3, NPCMSK2, PCINT19, PCIE2> D3;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD4, NPCMSK2, PCINT20, PCIE2> D4;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD5, NPCMSK2, PCINT21, PCIE2> D5;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD6, NPCMSK2, PCINT22, PCIE2> D6;
    typedef _Pin<NDDRD, NPORTD, NPIND, PD7, NPCMSK2, PCINT23, PCIE2> D7;

    typedef Pin::B3 SPI_MOSI;
    typedef Pin::B4 SPI_MISO;
    typedef Pin::B5 SPI_SCK;
    };

class Arduino
    {
public:
    static void init();
    
    static unsigned long millis();
    static unsigned long micros();

    static void delay(unsigned long ms)
        {
        unsigned long start = millis();
        
        while (millis() - start <= ms)
            ;
        }

    /** Use this function if the expression for ms is a constant.

        If ms cannot be calculated at compile time, gcc will drag in floating
        point support.
     */
    static void constantDelay(double ms) 
        {
        _delay_ms(ms);
        }
    
    static void delayMicroseconds(unsigned int us);

    /** Use this function if the expression for us is a constant 

        If us cannot be calculated at compile time, gcc will drag in floating
        point support.
     */
    static void constantDelayMicroseconds(double us)
        {
        _delay_us(us);
        }
    
    static void interrupts() { sei(); }
    static void noInterrupts() { cli(); }
    
    // The analog pins in Arduino numbering
    typedef Pin::C0 A0;
    typedef Pin::C1 A1;
    typedef Pin::C2 A2;
    typedef Pin::C3 A3;
    typedef Pin::C4 A4;
    
    // The digital pins in Arduino numbering
    typedef Pin::D0 D0;
    typedef Pin::D1 D1;
    typedef Pin::D2 D2;
    typedef Pin::D3 D3;
    typedef Pin::D4 D4;
    typedef Pin::D5 D5;
    typedef Pin::D6 D6;
    typedef Pin::D7 D7;
    typedef Pin::B0 D8;
    typedef Pin::B1 D9;
    typedef Pin::B2 D10;
    typedef Pin::B3 D11;
    typedef Pin::B4 D12;
    typedef Pin::B5 D13;
    
    volatile static unsigned long timer0_overflow_count;
    volatile static unsigned long timer0_clock_cycles;
    volatile static unsigned long timer0_millis;
    };

template <class Sck, class Miso, class Mosi, class Ss> class _SPI
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
            Arduino::delayMicroseconds(delay);
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

#endif // ARDUINO_MINUS_MINUS
