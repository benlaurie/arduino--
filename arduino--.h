// -*- mode: c++; indent-tabs-mode: nil; -*-

#ifndef ARDUINO_MINUS_MINUS
#define ARDUINO_MINUS_MINUS

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
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
    void operator&=(byte bits) { _SFR_IO8(reg) &= bits; }
    void operator|=(byte bits) { _SFR_IO8(reg) |= bits; }
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

template <class TCNT_, class OCRA_, class OCRB_, class TCCRA_, class TCCRB_,
          class TIFR_, class TIMSK_, byte toiex, byte ociexa, byte ocfxa, 
          byte ociexb, byte ocfxb> 
class _Timer
    {
public:

    static TCNT_ TCNT;
    static OCRA_ OCRA;
    static OCRB_ OCRB;
    static TCCRA_ TCCRA;
    static TCCRB_ TCCRB;
    static TIFR_ TIFR;
    static TIMSK_ TIMSK;

    static void reset() { TCNT = 0; }

    static void enableOverflowInterrupt() { TIMSK |= _BV(toiex); }
    static void disableOverflowInterrupt() { TIMSK &= ~_BV(toiex); }

    static void enableCompareInterruptA(typename OCRA_::value_t count) 
        {
        // clear compare interrupt flag
        TIFR &= _BV(ocfxa);
        OCRA = count;
        TIMSK |= _BV(ociexa); 
        }
    static void enableCompareInterruptA() {  TIMSK |= _BV(ociexa); }
    static void disableCompareInterruptA() { TIMSK &= ~_BV(ociexa); }

    static void enableCompareInterruptB(typename OCRB_::value_t count) 
        { 
        // clear compare interrupt flag
        TIFR &= _BV(ocfxb);
        OCRB = count;
        TIMSK |= _BV(ociexb);
        }
    static void enableCompareInterruptB() {  TIMSK |= _BV(ociexb); }
    static void disableCompareInterruptB() { TIMSK &= ~_BV(ociexb); }

    static void stop() { TCCRB &= ((1 << 5) - 1) << 3; }
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
        TCCRB &= ~_BV(WGM02); 
        wgm01(0);
        }

    //* Mode 1 */
    static void phaseCorrectPWM()
        {
        // clear WGM02
        TCCRB &= ~_BV(WGM02);
        wgm01(_BV(WGM00));
        }

    /** Mode 2 */
    static void clearTimerOnCompare()
        { 
        // clear WGM02
        TCCRB &= ~_BV(WGM02);
        wgm01(_BV(WGM01));
        }

    /** Mode 3 */
    static void fastPWM()
        { 
        // clear WGM02
        TCCRB &= ~_BV(WGM02);
        wgm01(_BV(WGM01) | _BV(WGM00));
        }

    /** Mode 5 */
    static void phaseCorrectPWMOCRA()
        { 
        // set WGM02
        TCCRB |= _BV(WGM02);
        wgm01(_BV(WGM00));
        }
    
    /** Mode 7 */
    static void fastPWMOCRA() 
        { 
        // set WGM02
        TCCRB |= _BV(WGM02);
        wgm01(_BV(WGM01) | _BV(WGM00));
        }

private:
    static void prescaler(byte pre)
        { 
        byte tmp = TCCRB & (((1 << 2) - 1) << 3);
        tmp |= pre;
        TCCRB = tmp;
        }

    static void wgm01(byte waveform)
        { 
        byte tmp = TCCRA & 3;
        tmp |= waveform;
        TCCRA = tmp;
        }
    };

template<typename T, class Timer> 
volatile T _Clock<T, Timer>::timer_overflow_count = 0;

template<typename T, class Timer> 
volatile uint16_t _Clock<T, Timer>::timer_fract = 0;

template<typename T, class Timer> 
volatile T _Clock<T, Timer>::timer_millis = 0;

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

template <byte ddr, byte port, byte in, byte bit, byte pcport, 
  byte pcbit, byte pcen> class _AnalogPin : 
    public _Pin<ddr, port, in, bit, pcport, pcbit, pcen>
    {
    // AREF, Internal Vref turned off 
    static const uint8_t REF_AREF = 0;
    // AVCC with external capacitor at AREF pin
    static const uint8_t REF_AVCC_EXT = 1;
    // Internal 1.1V Voltage Reference with external capacitor at AREF pin
    static const uint8_t REF_INT_1_1_REF = 3;
    
    static void analogStart(uint8_t reference = REF_AREF)
        {
        // set the analog reference (high two bits of ADMUX) and select the
        // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
        // to 0 (the default).
#if defined(ADMUX)
        ADMUX = (reference << 6) | (in & 0x07);
#endif
        // start the conversion
        ADCSRA |= (1 << (ADSC));
        }

    static int analogRead(uint8_t reference = REF_AREF)
        {
        analogStart(reference);
        
        // ADSC is cleared when the conversion finishes
        while (ADCSRA & (1 << ADSC))
            ;

        // we have to read ADCL first; doing so locks both ADCL
        // and ADCH until ADCH is read.  reading ADCL second would
        // cause the results of each conversion to be discarded,
        // as ADCL and ADCH would be locked when it completed.
        uint8_t low  = ADCL;
        uint8_t high = ADCH;

        // combine the two bytes
        return (high << 8) | low;
        }

    static void analogWrite(int val)
        {
        // We need to make sure the PWM output is enabled for those pins
        // that support it, as we turn it off when digitally reading or
        // writing with them.  Also, make sure the pin is in output mode
        // for consistenty with Wiring, which doesn't require a pinMode
        // call for the analog output pins.
        _Pin<ddr, port, in, bit, pcport, pcbit, pcen>::modeOutput();
        }
    };

class AVRBase
    {
public:
    static void interrupts() { sei(); }
    static void noInterrupts() { cli(); }
    };

/** Don't use this directly, use Clock16 or Clock32 instead
 */
template<typename timeres_t, class Timer> class _Clock
    {
public:
    typedef timeres_t time_res_t;

    _Clock()
        {
        // enable timer overflow interrupt
        Timer::enableOverflowInterrupt();
        }
    static timeres_t millis()
        {
        // disable interrupts while we read timer0_millis or we might get an
        // inconsistent value (e.g. in the middle of the timer0_millis++)
        ScopedInterruptDisable sid;
        return timer_millis;
        }

    static uint16_t micros()
        {
        uint8_t m;
        uint8_t t;
        ScopedInterruptDisable sid;

        t = Timer::TCNT;
        m = timer_overflow_count % (1 << TIMER16_MICRO_SCALE);

        if ((Timer::TIFR & _BV(TOV0)) && (t == 0))
            m++;

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

    volatile static timeres_t timer_overflow_count;
    volatile static uint16_t timer_fract;
    volatile static timeres_t timer_millis;
    };

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
    // PROGMEM variant
    static void write_P(Out *out, const char *str)
        {
        byte v;
		while ((v = pgm_read_byte(str++)))
            out->write(v);
        }
    };

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#include "defs/m328.h"
#endif

#endif // ARDUINO_MINUS_MINUS
