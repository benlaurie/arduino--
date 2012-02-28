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

template <class TCNT_, class OCRA_, class OCRB_, class TCCRA_, class TCCRB_,
          class TIFR_, class TIMSK_, byte toiex, byte ociexa, byte ocfxa, 
          byte ociexb, byte ocfxb, byte csx0, byte csx1, byte csx2,
          byte wgmx0, byte wgmx1, byte wgmx2> 
class _Timer
    {
public:

    typedef OCRA_ T_OCRA;
    typedef OCRB_ T_OCRB;

    static TCNT_ R_TCNT;
    static OCRA_ R_OCRA;
    static OCRB_ R_OCRB;
    static TCCRA_ R_TCCRA;
    static TCCRB_ R_TCCRB;
    static TIFR_ R_TIFR;
    static TIMSK_ R_TIMSK;

    static void reset() { R_TCNT = 0; }

    static void enableOverflowInterrupt() { R_TIMSK |= _BV(toiex); }
    static void disableOverflowInterrupt() { R_TIMSK &= ~_BV(toiex); }

    static void enableCompareInterruptA(typename OCRA_::value_t count) 
        {
        // clear compare interrupt flag
        R_TIFR &= _BV(ocfxa);
        R_OCRA = count;
        R_TIMSK |= _BV(ociexa); 
        }
    static void enableCompareInterruptA() {  R_TIMSK |= _BV(ociexa); }
    static void disableCompareInterruptA() { R_TIMSK &= ~_BV(ociexa); }

    static void enableCompareInterruptB(typename OCRB_::value_t count) 
        { 
        // clear compare interrupt flag
        R_TIFR &= _BV(ocfxb);
        R_OCRB = count;
        R_TIMSK |= _BV(ociexb);
        }
    static void enableCompareInterruptB() {  R_TIMSK |= _BV(ociexb); }
    static void disableCompareInterruptB() { R_TIMSK &= ~_BV(ociexb); }

    static void stop() { R_TCCRB &= ((1 << 5) - 1) << 3; }
    static void prescaler1() { prescaler(_BV(csx0)); }
    static void prescaler8() { prescaler(_BV(csx1)); }
    static void prescaler64() { prescaler(_BV(csx1) | _BV(csx0)); }
    static void prescaler256() { prescaler(_BV(csx2)); }
    static void prescaler1024() { prescaler(_BV(csx2) | _BV(csx0)); }

    static void externalFalling() { prescaler(_BV(csx2) | _BV(csx1)); }
    static void externalRising() 
        { prescaler(_BV(csx2) | _BV(csx1) | _BV(csx0)); }


    //** Mode 0 */
    static void normal() 
        { 
        // clear WGMx2
        R_TCCRB &= ~_BV(wgmx2);
        wgm01(0);
        }

    //* Mode 1 */
    static void phaseCorrectPWM()
        {
        // clear WGMx2
        R_TCCRB &= ~_BV(wgmx2);
        wgm01(_BV(wgmx0));
        }

    /** Mode 2 */
    static void clearTimerOnCompare()
        { 
        // clear WGMx2
        R_TCCRB &= ~_BV(wgmx2);
        wgm01(_BV(wgmx1));
        }

    /** Mode 3 */
    static void fastPWM()
        { 
        // clear WGMx2
        R_TCCRB &= ~_BV(wgmx2);
        wgm01(_BV(wgmx1) | _BV(wgmx0));
        }

    /** Mode 5 */
    static void phaseCorrectPWMOCRA()
        { 
        // set WGMx2
        R_TCCRB |= _BV(wgmx2);
        wgm01(_BV(wgmx0));
        }
    
    /** Mode 7 */
    static void fastPWMOCRA() 
        { 
        // set WGMx2
        R_TCCRB |= _BV(wgmx2);
        wgm01(_BV(wgmx1) | _BV(wgmx0));
        }

private:
    static void prescaler(byte pre)
        { 
        byte tmp = R_TCCRB & (((1 << 2) - 1) << 3);
        tmp |= pre;
        R_TCCRB = tmp;
        }

    static void wgm01(byte waveform)
        { 
        byte tmp = R_TCCRA & 3;
        tmp |= waveform;
        R_TCCRA = tmp;
        }
    };

/*
  We are forcing gcc to inline the _Pin methods. I think this shouldn't be 
  necessary, but when the _Pin methods are used via a subclass like 
  _ChangeInterruptPin, gcc doesn't automatically inline these methods any more.

  This might be a bug in the inliner, but to be fair, we are mixing template 
  arguments with inheritance.
 */
template <byte ddr, byte port, byte in, byte bit> 
class _Pin
    {
public:

    static void modeOutput() __attribute__((always_inline))
        { _SFR_IO8(ddr) |= _BV(bit); }
    static void modeInput() __attribute__((always_inline))
        { _SFR_IO8(ddr) &= ~_BV(bit); }
    static void modeInputPullup() __attribute__((always_inline))
        { modeInput(); set(); }
    static void set() __attribute__((always_inline))
        { _SFR_IO8(port) |= _BV(bit); }
    static void clear() __attribute__((always_inline))
        { _SFR_IO8(port) &= ~_BV(bit); }

    /** Return 1 if the Pin reads HIGH */
    static byte read() __attribute__((always_inline))
        { return !!(_SFR_IO8(in) & _BV(bit)); }
    static byte toggle() __attribute__((always_inline))
        { return (_SFR_IO8(port) ^= _BV(bit)); }
    };

template <class Pin_, class Timer_, class OCR_, byte tccrc, byte foc>
class _PWMPin : public Pin_
    {
    static Timer_ Timer;
    static OCR_ R_OCR;

    /** Force the comparison of the output compare register.

        Advanced functionality: This avoids a glitch in PWM generation
        when the clock associated with this PWM Pin is not yet in a
        PWM mode. 

        This method should not be called if the clock is already in PWM mode.
     */
    static void forceCompare() __attribute__((always_inline))
        {
        _SFR_IO8(tccrc) |= foc;
        }

    static void pwmWrite(byte value) __attribute__((always_inline))
        {
        R_OCR = value;
        }
    };

// pcicr is the interrupy control register address, pcen is the enable bit, 
// pcmsk is pin change mask register and pcbit the value bit
template <class Pin_, byte pcicr, byte pcen, byte pcmsk, byte pcbit> 
class _ChangeInterruptPin : public Pin_
    {
public:

    static void enableChangeInterrupt() __attribute__((always_inline))
        {
        _SFR_IO8(pcicr) |= _BV(pcen);
        _SFR_IO8(pcmsk) |= _BV(pcbit); 
        }
    static void disableChangeInterrupt() __attribute__((always_inline))
        { 
        if (_SFR_IO8(pcmsk) &= ~_BV(pcbit))
            _SFR_IO8(pcicr) &= ~_BV(pcen);
        }
    };

#if defined (ADMUX) && defined (ADCSRA) && defined (ADSC) && defined (ADCH) \
  && defined (ADCL)

template <class Pin_, byte ain>
class _AnalogPin : public Pin_
    {
    static void analogStart(uint8_t reference) __attribute__((always_inline))
        {
        // set the analog reference (high two bits of ADMUX) and select the
        // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
        // to 0 (the default).
        ADMUX = (reference << 6) | (ain & 0x07);
        // start the conversion
        ADCSRA |= (1 << (ADSC));
        }

    static int analogRead(uint8_t reference)
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
    };

#endif

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
        // disable interrupts while we read timer0millis or we might get an
        // inconsistent value (e.g. in the middle of the timer_millis++)
        ScopedInterruptDisable sid;
        return timer_millis;
        }

    static uint16_t micros()
        {
        uint8_t m;
        uint8_t t;
        ScopedInterruptDisable sid;

        t = Timer::R_TCNT;
        m = timer_overflow_count % (1 << TIMER16_MICRO_SCALE);

        if ((Timer::R_TIFR & _BV(TOV0)) && (t == 0))
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

template<typename timeres_t, class Timer> 
volatile timeres_t _Clock<timeres_t, Timer>::timer_overflow_count = 0;

template<typename timeres_t, class Timer> 
volatile uint16_t _Clock<timeres_t, Timer>::timer_fract = 0;

template<typename timeres_t, class Timer> 
volatile timeres_t _Clock<timeres_t, Timer>::timer_millis = 0;

/** busy wait */
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

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) \
  || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__)    \
  || defined (__AVR_ATmega168P__)
#include "defs/mx8.h"
#elif defined (__AVR_ATtiny85__) || defined (__AVR_ATtiny45__) \
  || defined (__AVR_ATTiny25__)
#include "defs/tnx5.h"
#else
#error "No MCU specific definitions found"
#endif

#endif // ARDUINO_MINUS_MINUS
