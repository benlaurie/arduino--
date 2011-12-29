// -*- mode: c++; indent-tabs-mode: nil; -*-

#ifndef ARDUINO_MINUS_MINUS
#define ARDUINO_MINUS_MINUS

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr-ports.h"

typedef uint8_t byte;

template <byte reg> class _Register
    {
 public:
    static void set(byte bit) { _SFR_IO8(reg) |= _BV(bit); }
    static void clear(byte bit) { _SFR_IO8(reg) &= ~_BV(bit); }
    void operator=(byte bits) { _SFR_IO8(reg) = bits; }
    operator byte() const { return _SFR_IO8(reg); }
    };

// FIXME: should Pins also be static members instead of typedefs?
// These can't be typedefs because then we can't define operator= on them.
class Register
    {
 public:
#undef EICRA
    static _Register<NEICRA> EICRA;
#undef EIMSK
    static _Register<NEIMSK> EIMSK;
#undef SPCR
    static _Register<NSPCR> SPCR;
    };

template <byte lsb, byte maskbit> class _Interrupt
    {
 public:
    enum Mode
        {
        LOW = 0,
        CHANGE = 1,
        FALLING_EDGE = 2,
        RISING_EDGE = 3
        };
    static void enable(Mode mode)
        {
        Register::EICRA = (Register::EICRA & ~(3 << lsb)) | (mode << lsb);
        Register::EIMSK.set(maskbit);
        }
    static void disable()
        { Register::EIMSK.clear(maskbit); }
    };

typedef class _Interrupt<ISC00, INT0> Interrupt0;
typedef class _Interrupt<ISC10, INT1> Interrupt1;

template <byte ddr, byte port, byte in, byte bit, byte pcport, 
  byte pcbit, byte pcen> class _Pin
    {
public:

    static void enablePCInterrupt() 
        {
        PCICR |= _BV(pcen);
        _SFR_IO8(pcport) |= _BV(pcbit); 
        }
    static void disablePCInterrupt() 
        { 
        if (_SFR_IO8(pcport) &= ~_BV(pcbit))
            PCICR &= ~_BV(pcen);
        }

    static void modeOutput() { _SFR_IO8(ddr) |= _BV(bit); }
    static void modeInput() { _SFR_IO8(ddr) &= ~_BV(bit); }
    static void set() { _SFR_IO8(port) |= _BV(bit); }
    static void clear() { _SFR_IO8(port) &= ~_BV(bit); }

    /** Return 1 if the Pin reads HIGH */
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

    typedef Pin::B2 SPI_SS;
    typedef Pin::B3 SPI_MOSI;
    typedef Pin::B4 SPI_MISO;
    typedef Pin::B5 SPI_SCK;
    };

/** Don't use this directly, use Arduino16 or Arduino32 instead
 */
template<typename timeres_t>
class _Arduino
    {
public:

    typedef timeres_t time_res_t;

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
    
    static void interrupts() { sei(); }
    static void noInterrupts() { cli(); }
    
    static void init()
        {
        // this needs to be called before setup() or some functions won't
        // work there
        sei();
    
        // on the ATmega168, timer 0 is also used for fast hardware pwm
        // (using phase-correct PWM would mean that timer 0 overflowed half as 
        // often, resulting in different millis() behavior on the ATmega8 and 
        //ATmega168)
#if !defined(__AVR_ATmega8__)
        _SFR_BYTE(TCCR0A) |= (_BV(WGM01) | _BV(WGM00));
#endif  
        // set timer 0 prescale factor to 64
#if defined(__AVR_ATmega8__)
        _SFR_BYTE(TCCR0) |= (_BV(CS01) | _BV(CS00));
#else
        _SFR_BYTE(TCCR0B) |= (_BV(CS01) | _BV(CS00));
#endif
        // enable timer 0 overflow interrupt
#if defined(__AVR_ATmega8__)
        _SFR_BYTE(TIMSK) |= _BV(TOIE0);
#else
        _SFR_BYTE(TIMSK0) |= _BV(TOIE0);
#endif

        // timers 1 and 2 are used for phase-correct hardware pwm
        // this is better for motors as it ensures an even waveform
        // note, however, that fast pwm mode can achieve a frequency of up
        // 8 MHz (with a 16 MHz clock) at 50% duty cycle

        // set timer 1 prescale factor to 64
        _SFR_BYTE(TCCR1B) |= (_BV(CS11) | _BV(CS10));
        // put timer 1 in 8-bit phase correct pwm mode
        _SFR_BYTE(TCCR1A) |= _BV(WGM10);

    // set timer 2 prescale factor to 64
#if defined(__AVR_ATmega8__)
        _SFR_BYTE(TCCR2) |= _BV(CS22);
#else
        _SFR_BYTE(TCCR2B) |= _BV(CS22);
#endif
    // configure timer 2 for phase correct pwm (8-bit)
#if defined(__AVR_ATmega8__)
        _SFR_BYTE(TCCR2) |= _BV(WGM20);
#else
        _SFR_BYTE(TCCR2A) |= _BV(WGM20);
#endif

#if defined(__AVR_ATmega1280__)
        // set timer 3, 4, 5 prescale factor to 64
        _SFR_BYTE(TCCR3B) |= (_BV(CS31) | _BV(CS30));
        _SFR_BYTE(TCCR4B) |= (_BV(CS41) | _BV(CS40));
        _SFR_BYTE(TCCR5B) |= (_BV(CS51) | _BV(CS50));
        // put timer 3, 4, 5 in 8-bit phase correct pwm mode
        _SFR_BYTE(TCCR3A) |= _BV(WGM30);
        _SFR_BYTE(TCCR4A) |= _BV(WGM40);
        _SFR_BYTE(TCCR5A) |= _BV(WGM50);
#endif

        // set a2d prescale factor to 128
        // 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
        // XXX: this will not work properly for other clock speeds, and
        // this code should use F_CPU to determine the prescale factor.
        _SFR_BYTE(ADCSRA) |= (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0));

        // enable a2d conversions
        _SFR_BYTE(ADCSRA) |= _BV(ADEN);

        // the bootloader connects pins 0 and 1 to the USART; disconnect them
        // here so they can be used as normal digital i/o; they will be
        // reconnected in Serial.begin()
#if defined(__AVR_ATmega8__)
        UCSRB = 0;
#else
        UCSR0B = 0;
#endif
        }
    
    static timeres_t millis()
        {
        const uint8_t oldSREG = SREG;
    
        // disable interrupts while we read timer0_millis or we might get an
        // inconsistent value (e.g. in the middle of the timer0_millis++)
        cli();
        const timeres_t m = timer0_millis;
        SREG = oldSREG;
        
        return m;
        }

    static timeres_t micros()
        {
        timeres_t m;
        uint16_t t;
        uint8_t oldSREG = SREG;
    
        cli();
        t = TCNT0;
  
#ifdef TIFR0
        if ((TIFR0 & _BV(TOV0)) && (t == 0))
            t = 256;
#else
        if ((TIFR & _BV(TOV0)) && (t == 0))
            t = 256;
#endif

        m = timer0_overflow_count;
        SREG = oldSREG;
    
        return ((m << 8) + t) * 64 / (F_CPU / 1000000L);
        }

    static void delay(timeres_t ms)
        {
        const timeres_t start = millis();
        
        while (millis() - start <= ms)
            ;
        }

    volatile static timeres_t timer0_overflow_count;
    volatile static timeres_t timer0_clock_cycles;
    volatile static timeres_t timer0_millis;
    };

template<typename T> volatile T _Arduino<T>::timer0_overflow_count = 0;
template<typename T> volatile T _Arduino<T>::timer0_clock_cycles = 0;
template<typename T> volatile T _Arduino<T>::timer0_millis = 0;

/** This is an Arduino with 16bit timer resolution.
    
    The value from Arduino16::millis() will wrap around after about 65 seconds
    and the value from Arduino16::micros() will wrap around after about 65 ms.

    When you use Arduino16, you must #include "arduino++timer16.h", or 
    Arduino16::millis() will always return 0.

    The recommended way to use a timer value in user code is:

    typename Arduino16::time_res_t now = Arduino16::millis();

    Motivation: With Arduino16 instead of Arduino32, Lars has seen a code size 
    reduction of 238 bytes with avr-gcc 4.6.1.
 */
typedef _Arduino<uint16_t> Arduino16;

/** This is an Arduino with 32bit timer resolution.
    
    When you use Arduino32, you must #include "arduino++timer32.h", or 
    Arduino32::millis() will always return 0.

    The recommended way to use a timer value in user code is:

    typename Arduino32::time_res_t now = Arduino32::millis();

    The value from Arduino32::millis() will wrap around after about 49 days
    and the value from Arduino32::micros() will wrap around after about 
    71 minutes.
 */
typedef _Arduino<uint32_t> Arduino32;

void delayMicroseconds(unsigned int us)
    {
    // calling avrlib's delay_us() function with low values (e.g. 1 or
    // 2 microseconds) gives delays longer than desired.
    //delay_us(us);

#if F_CPU >= 16000000L
    // for the 16 MHz clock on most Arduino boards
    
    // for a one-microsecond delay, simply return.  the overhead
    // of the function call yields a delay of approximately 1 1/8 us.
    if (--us == 0)
        return;

    // the following loop takes a quarter of a microsecond (4 cycles)
    // per iteration, so execute it four times for each microsecond of
    // delay requested.
    us <<= 2;

    // account for the time taken in the preceeding commands.
    us -= 2;
#else
    // for the 8 MHz internal clock on the ATmega168

    // for a one- or two-microsecond delay, simply return.  the overhead of
    // the function calls takes more than two microseconds.  can't just
    // subtract two, since us is unsigned; we'd overflow.
    if (--us == 0)
        return;
    if (--us == 0)
        return;

    // the following loop takes half of a microsecond (4 cycles)
    // per iteration, so execute it twice for each microsecond of
    // delay requested.
    us <<= 1;
    
    // partially compensate for the time taken by the preceeding commands.
    // we can't subtract any more than this or we'd overflow w/ small delays.
    us--;
#endif

    // disable interrupts, otherwise the timer 0 overflow interrupt that
    // tracks milliseconds will make us delay longer than we want.
    const uint8_t oldSREG = SREG;
    cli();

    // busy wait
    __asm__ __volatile__ (
              "1: sbiw %0,1" "\n\t" // 2 cycles
              "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
              );

    // reenable interrupts.
    SREG = oldSREG;
    }

template <class Out> class HexWriter
    {
public:
    static void write(Out *out, byte b)
        {
        writeNibble(out, b >> 4);
        writeNibble(out, b & 0x0f);
        }
    static void writeNibble(Out *out, byte b)
        {
        if (b < 10)
            out->write(b + '0');
        else
            out->write(b + 'a' - 10);
        }
    };

#endif // ARDUINO_MINUS_MINUS
