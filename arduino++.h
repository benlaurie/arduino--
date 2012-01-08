// -*- mode: c++; indent-tabs-mode: nil; -*-

#ifndef ARDUINO_MINUS_MINUS
#define ARDUINO_MINUS_MINUS

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "avr-ports.h"

typedef uint8_t byte;

class ScopedInterruptDisable
    {
public:
    ScopedInterruptDisable() : sreg_(SREG) { cli(); }
    ~ScopedInterruptDisable() { SREG = sreg_; }

private:
    byte sreg_;
    };

template <byte reg> class _Register
    {
 public:    
    typedef byte value_t;

    static void set(byte bit) { _SFR_IO8(reg) |= _BV(bit); }
    static void clear(byte bit) { _SFR_IO8(reg) &= ~_BV(bit); }
    static byte atomicRead()
        {
        ScopedInterruptDisable sid;
        return _SFR_IO8(reg);
        }
    static void atomicWrite(byte val)
        {
        ScopedInterruptDisable sid;
        _SFR_IO8(reg) = val;
        }
    void operator=(byte bits) { _SFR_IO8(reg) = bits; }
    operator byte() const { return _SFR_IO8(reg); }
    };

template <byte reg> class _Register16
    {
 public:
    typedef uint16_t value_t;

    static void setHigh(byte bit) { _SFR_IO8(reg + 1) |= _BV(bit); }
    static void setLow(byte bit) { _SFR_IO8(reg) |= _BV(bit); }
    static void clearHigh(byte bit) { _SFR_IO8(reg + 1) &= ~_BV(bit); }
    static void clearLow(byte bit) { _SFR_IO8(reg) &= ~_BV(bit); }
    static uint16_t atomicRead()
        {
        ScopedInterruptDisable sid;
        return _SFR_IO16(reg);
        }
    static void atomicWrite(uint16_t val)
        {
        ScopedInterruptDisable sid;
        _SFR_IO16(reg) = val;
        }

    byte readLow() { return _SFR_IO8(reg); };
    byte readHigh() { return _SFR_IO8(reg + 1); };

    void operator=(uint16_t bits) { _SFR_IO16(reg) = bits; }
    operator uint16_t() const { return _SFR_IO16(reg); }
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

template <class TCNT_, class OCRA_, class OCRB_, byte tccrxa, byte tccrxb,
          byte tifrx, byte timskx, byte toiex, byte ociexa, byte ocfxa, 
          byte ociexb, byte ocfxb> 
class _Timer
    {
public:

    static TCNT_ TCNT;
    static OCRA_ OCRA;
    static OCRB_ OCRB;

    static void reset() { TCNT = 0; }

    static void enableOverflowInterrupt() { _SFR_IO8(timskx) |= _BV(toiex); }
    static void disableOverflowInterrupt() { _SFR_IO8(timskx) &= ~_BV(toiex); }

    static void enableCompareInterruptA(typename OCRA_::value_t count) 
        {
        // clear compare interrupt flag
        _SFR_IO8(tifrx) &= _BV(ocfxa);
        OCRA = count;
        _SFR_IO8(timskx) |= _BV(ociexa); 
        }
    static void enableCompareInterruptA() {  _SFR_IO8(timskx) |= _BV(ociexa); }
    static void disableCompareInterruptA() { _SFR_IO8(timskx) &= ~_BV(ociexa); }

    static void enableCompareInterruptB(typename OCRB_::value_t count) 
        { 
        // clear compare interrupt flag
        _SFR_IO8(tifrx) &= _BV(ocfxb);
        OCRB = count;
        _SFR_IO8(timskx) |= _BV(ociexb);
        }
    static void enableCompareInterruptB() {  _SFR_IO8(timskx) |= _BV(ociexb); }
    static void disableCompareInterruptB() { _SFR_IO8(timskx) &= ~_BV(ociexb); }

    static void stop() { _SFR_IO8(tccrxb) &= ((1 << 5) - 1) << 3; }
    static void prescaler1() { prescaler(_BV(CS00)); }
    static void prescaler8() { prescaler(_BV(CS01)); }
    static void prescaler64() { prescaler(_BV(CS01) | _BV(CS00)); }
    static void prescaler256() { prescaler(_BV(CS02)); }
    static void prescaler1024() { prescaler(_BV(CS02) | _BV(CS00)); }

    static void externalFalling() { prescaler(_BV(CS02) | _BV(CS01)); }
    static void externalRising() 
        { prescaler(_BV(CS02) | _BV(CS01) | _BV(CS00)); }


    //** Mode 0 */
    static void normal() 
        { 
        _SFR_IO8(tccrxb) &= ~_BV(WGM02); 
        wgm01(0);
        }

    //* Mode 1 */
    static void phaseCorrectPWM()
        {
        // clear WGM02
        _SFR_IO8(tccrxb) &= ~_BV(WGM02);
        wgm01(_BV(WGM00));
        }

    /** Mode 2 */
    static void clearTimerOnCompare()
        { 
        // clear WGM02
        _SFR_IO8(tccrxb) &= ~_BV(WGM02);
        wgm01(_BV(WGM01));
        }

    /** Mode 3 */
    static void fastPWM()
        { 
        // clear WGM02
        _SFR_IO8(tccrxb) &= ~_BV(WGM02);
        wgm01(_BV(WGM01) | _BV(WGM00));
        }

    /** Mode 5 */
    static void phaseCorrectPWMOCRA()
        { 
        // set WGM02
        _SFR_IO8(tccrxb) |= _BV(WGM02);
        wgm01(_BV(WGM00));
        }
    
    /** Mode 7 */
    static void fastPWMOCRA() 
        { 
        // set WGM02
        _SFR_IO8(tccrxb) |= _BV(WGM02);
        wgm01(_BV(WGM01) | _BV(WGM00));
        }

private:
    static void prescaler(byte pre)
        { 
        byte tmp = _SFR_IO8(tccrxb) & (((1 << 2) - 1) << 3);
        tmp |= pre;
        _SFR_IO8(tccrxb) = tmp;
        }

    static void wgm01(byte waveform)
        { 
        byte tmp = _SFR_IO8(tccrxa) & 3;
        tmp |= waveform;
        _SFR_IO8(tccrxa) = tmp;
        }
    };

// make sure that the CS bit value definitions are the same for every timer 
// register we are interested in
#if ((CS00 != CS10) || (CS01 != CS11) || (CS02 != CS12))
#error "Timer CSxx register constants violate assumptions in _Timer"
#endif 

// make sure that the WGM bit value definitions are the same for every timer 
// register we are interested in
#if ((WGM00 != WGM10) || (WGM01 != WGM11) || (WGM02 != WGM12))
#error "Timer WGMxx register constants violate assumptions in _Timer"
#endif 

typedef _Timer<_Register<NTCNT0>, _Register<NOCR0A>, _Register<NOCR0B>, 
               NTCCR0A, NTCCR0B, NTIFR0, NTIMSK0, TOIE0, OCIE0A, OCF0A,
               OCIE0B, OCF0B> Timer0;

typedef _Timer<_Register16<NTCNT1>, _Register16<NOCR1A>, _Register16<NOCR1B>,
               NTCCR1A, NTCCR1B, NTIFR1, NTIMSK1, TOIE1, OCIE1A, OCF1A, 
               OCIE1B, OCF1B> Timer1;

typedef _Timer<_Register<NTCNT2>, _Register<NOCR2A>, _Register<NOCR2B>,
               NTCCR2A, NTCCR2B, NTIFR2, NTIMSK2, TOIE2, OCIE2A, OCF2A, 
               OCIE2B, OCF2B> Timer2;

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
    static void modeInputPullup() { modeInput(); set(); }
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

class AVRBase
    {
public:

    static void interrupts() { sei(); }
    static void noInterrupts() { cli(); }
    };

class Arduino : public AVRBase
    {
public:

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
    
    static void init()
        {
        // this needs to be called before setup() or some functions won't
        // work there
        sei();
    
        // on the ATmega168, timer 0 is also used for fast hardware pwm
        // (using phase-correct PWM would mean that timer 0 overflowed half as 
        // often, resulting in different millis() behavior on the ATmega8 and 
        // ATmega168)
#if !defined(__AVR_ATmega8__)
        Timer0::fastPWM();
#endif  
        // set timer 0 prescale factor to 64

        Timer0::prescaler64();

        // Note: timer 0 interrupt is _NOT_ enabled. To enable it,
        // include one of the clock headers.

        // timers 1 and 2 are used for phase-correct hardware pwm
        // this is better for motors as it ensures an even waveform
        // note, however, that fast pwm mode can achieve a frequency of up
        // 8 MHz (with a 16 MHz clock) at 50% duty cycle

        // set timer 1 prescale factor to 64
        Timer1::prescaler64();
        // put timer 1 in 8-bit phase correct pwm mode
        Timer1::phaseCorrectPWM();

        // set timer 2 prescale factor to 64
        Timer2::prescaler64();

    // configure timer 2 for phase correct pwm (8-bit)
#if defined(__AVR_ATmega8__)
        _SFR_BYTE(TCCR2) |= _BV(WGM20);
#else
        Timer2::phaseCorrectPWM();
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
    };

/** Don't use this directly, use Clock16 or Clock32 instead
 */
template<typename timeres_t> class _Clock
    {
public:
    typedef timeres_t time_res_t;

    _Clock()
        {
        // enable timer 0 overflow interrupt
        Timer0::enableOverflowInterrupt();
        }
    static timeres_t millis()
        {
        // disable interrupts while we read timer0_millis or we might get an
        // inconsistent value (e.g. in the middle of the timer0_millis++)
        ScopedInterruptDisable sid;
        return timer0_millis;
        }

    static uint16_t micros()
        {
        uint8_t m;
        uint8_t t;
        ScopedInterruptDisable sid;

        t = TCNT0;
        m = timer0_overflow_count % (1 << TIMER16_MICRO_SCALE);
  
#ifdef TIFR0
        if ((TIFR0 & _BV(TOV0)) && (t == 0))
            m++;
#else
        if ((TIFR & _BV(TOV0)) && (t == 0))
            m++;
#endif

        return ((m << 8) + t) * (64 / (F_CPU / 1000000L));
        }

    static void delay(timeres_t ms)
        {
        const timeres_t start = millis();
        
        while (millis() - start <= ms)
            ;
        }

    static void sleep(timeres_t ms)
        {
        const timeres_t start = millis();
        
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
#ifdef sleep_bod_disable
        sleep_bod_disable();
#endif
        while (millis() - start <= ms)
            {
            sei();
            sleep_cpu();
            }
        
        sei();
        sleep_disable();
        }

    volatile static timeres_t timer0_overflow_count;
    volatile static uint16_t timer0_fract;
    volatile static timeres_t timer0_millis;
    };

template<typename T> volatile T _Clock<T>::timer0_overflow_count = 0;
template<typename T> volatile uint16_t _Clock<T>::timer0_fract = 0;
template<typename T> volatile T _Clock<T>::timer0_millis = 0;

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

    {
    // disable interrupts, otherwise the timer 0 overflow interrupt that
    // tracks milliseconds will make us delay longer than we want.
    ScopedInterruptDisable sid;

    // busy wait
    __asm__ __volatile__ (
              "1: sbiw %0,1" "\n\t" // 2 cycles
              "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
              );

    }
    }

template <class Out> class HexWriter
    {
public:
    static void write(Out *out, byte b)
        {
        writeNibble(out, b >> 4);
        writeNibble(out, b & 0x0f);
        }
    static void write(Out *out, uint16_t i)
        {
        write(out, static_cast<byte>(i >> 8));
        write(out, static_cast<byte>(i & 0xff));
        }
    static void writeNibble(Out *out, byte b)
        {
        if (b < 10)
            out->write(b + '0');
        else
            out->write(b + 'a' - 10);
        }
    };

template <class Out> class StringWriter
    {
public:
    static void write(Out *out, const char *str)
        {
        while (*str)
            out->write(*str++);
        }
    };

#endif // ARDUINO_MINUS_MINUS
