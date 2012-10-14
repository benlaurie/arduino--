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

    static void write(byte val) { _SFR_IO8(reg) = val; }
    static byte read() { return _SFR_IO8(reg); }

    static void writeAnd(byte val) { _SFR_IO8(reg) &= val; }
    static void writeOr(byte val) { _SFR_IO8(reg) |= val; }

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

    static uint16_t read() { return _SFR_IO16(reg); }
    static void write(uint16_t val) { _SFR_IO16(reg) = val; }

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

    void operator&=(byte bits) { _SFR_IO8(reg) &= bits; }
    void operator|=(byte bits) { _SFR_IO8(reg) |= bits; }
    void operator=(uint16_t bits) { _SFR_IO16(reg) = bits; }
    operator uint16_t() const { return _SFR_IO16(reg); }
    };

/** An Output comparator, part of a timer

    When new Timer types need to be implemented, make sure that:

    * OCFx_ is on TIFR_
    * OCIEx_ is on TIMSK_
    * COM0_ and COM1_ are on TCCR_
    * FOC_ is on TCCRF_
*/
template <class OCR_, class TCCR_, byte COM1_, byte COM0_,
          class TIMSK_,  byte OCIEx_,
          class TIFR_, byte OCFx_,
          class TCCRF_, byte FOC_>
class _OutputComparator
    {
public:
    static typename OCR_::value_t read() { return OCR_::read(); }
    static void write(typename OCR_::value_t val) { OCR_::write(val); }

    static void enableInterrupt(typename OCR_::value_t count)
        {
        // clear compare interrupt flag
        TIFR_::set(OCFx_);
        OCR_::write(count);
        TIMSK_::set(OCIEx_);
        }

    static void enableInterrupt() {  TIMSK_::set(OCIEx_); }
    static void disableInterrupt() { TIMSK_::clear(OCIEx_); }

    static void modeToggle() { TCCR_::set(COM0_); }
    static void modeClear() { TCCR_::set(COM1_); }
    static void modeSet() { TCCR_::write(_BV(COM1_) | _BV(COM0_)); }
    static void disable()
        { TCCR_::writeAnd(~(_BV(COM0_) | _BV(COM1_))); }

    /** Force the comparison of the output compare register.

        Advanced functionality: This avoids a glitch in PWM generation
        when the clock associated with this PWM Pin is not yet in a
        PWM mode.

        This method should not be called if the timer is already in
        PWM mode.

        So don't call this if you have called Arduino::init before.
    */
    static void forceCompare() __attribute__((always_inline))
        {
        TCCRF_::set(FOC_);
        }
    };

/** A basic Timer with the counter register and overflow interrupt.
 */
template <class TCNT_, class TIMSK_, byte TOIE_>
class _Timer
    {
public:
    static void reset() { TCNT_::write(0); }
    static typename TCNT_::value_t read() { return TCNT_::read(); }
    static void write(typename TCNT_::value_t val) { TCNT_::write(val); }

    static void enableOverflowInterrupt() { TIMSK_::set(TOIE_); }
    static void disableOverflowInterrupt() { TIMSK_::clear(TOIE_); }

    static void stop() { prescaler(0); }

    typedef TCNT_ TCNT;
private:

    // must be implemented by subclasses
    static void prescaler(byte pre);
    };

/** Hardware timer with 2 output compare units and 2 config registers
    (TCCRA_ and TCCRB_)

  This timer unit is used by the following devices: ATMega48/88/168/328
  (Timer0 and Timer2)

  The TCCRA_ register contains the following configuration bits (x=#timer):

  @verbatim
  +------+------+------+------+---+---+-----+-----+
  |COMxA1|COMxA0|COMxB1|COMxB0| - | - |WGMx1|WGMx0|
  +------+------+------+------+---+---+-----+-----+
  @endverbatim

  The TCCRB_ register contains the following configuration bits (x=#timer):

  @verbatim
  +-----+-----+---+---+-----+----+----+----+
  |FOCxA|FOCxB| - | - |WGMx2|CSx2|CSx1|CSx0|
  +-----+-----+---+---+-----+----+----+----+
  @endverbatim

  The TIMSK_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+---+---+---+---+------+------+-----+
  | - | - | - | - | - |OCIExB|OCIExA|TOIEx|
  +---+---+---+---+---+------+------+-----+
  @endverbatim

  The TIFR_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+---+---+---+---+-----+-----+----+
  | - | - | - | - | - |OCFxB|OCFxA|TOVx|
  +---+---+---+---+---+-----+-----+----+
  @endverbatim
*/
template <class TCNT_, class OCRA_, class OCRB_, class TCCRA_, class TCCRB_,
          class TIMSK_, class TIFR_>
class _Timer_2C : public _Timer<TCNT_, TIMSK_, 0> // 0 is TOIEx
    {
public:

    // TCCRA_
    static const byte COMxA1 = 7, COMxA0 = 6, COMxB1 = 5, COMxB0 = 4;
    static const byte WGMx1 = 1, WGMx0 = 0;

    // TCCRB_
    static const byte FOCxA = 7, FOCxB = 6;
    static const byte WGMx2 = 3;
    static const byte CSx2 = 2, CSx1 = 1, CSx0 = 0;

    // TIMSK_
    static const byte OCIExB = 2, OCIExA = 1;
    static const byte TOIEx = 0;

    // TIFR_
    static const byte OCFxB = 2, OCFxA = 1;
    static const byte TOVx = 0;

    typedef _OutputComparator<OCRA_, TCCRA_, COMxA1, COMxA0, TIMSK_, OCIExA,
                              TIFR_, OCFxA, TCCRB_, FOCxA>
    CompA;

    typedef _OutputComparator<OCRB_, TCCRA_, COMxB1, COMxB0, TIMSK_, OCIExB,
                              TIFR_, OCFxB, TCCRB_, FOCxB>
    CompB;

    typedef TIFR_ TIFR;

    static void prescaler1() { prescaler(_BV(CSx0)); }
    static void prescaler8() { prescaler(_BV(CSx1)); }
    static void prescaler64() { prescaler(_BV(CSx1) | _BV(CSx0)); }
    static void prescaler256() { prescaler(_BV(CSx2)); }
    static void prescaler1024() { prescaler(_BV(CSx2) | _BV(CSx0)); }

    static void externalFalling() { prescaler(_BV(CSx2) | _BV(CSx1)); }
    static void externalRising()
        { prescaler(_BV(CSx2) | _BV(CSx1) | _BV(CSx0)); }

    static void modeNormal() { wgm(0); }
    static void modePhaseCorrectPWM() { wgm(_BV(WGMx0)); }
    static void modeClearTimerOnCompare() { wgm(_BV(WGMx1)); }
    static void modeFastPWM() { wgm(_BV(WGMx1) | _BV(WGMx0)); }
    static void modePhaseCorrectPWMOCRA() { wgm(_BV(WGMx2) | _BV(WGMx0)); }
    static void modeFastPWMOCRA() { wgm(_BV(WGMx2) | _BV(WGMx1) | _BV(WGMx0)); }

private:
    static void prescaler(byte pre)
        {
        byte tmp = TCCRB_::read() & ~7;
        TCCRB_::write(tmp | pre);
        }

    static void wgm(byte waveform)
        {
        byte tmpb = TCCRB_::read() & ~_BV(WGMx2);
        tmpb |= waveform & _BV(WGMx2);
        TCCRB_::write(tmpb);

        const byte mask10 = _BV(WGMx1) | _BV(WGMx0);
        byte tmpa = TCCRA_::read() & ~mask10;
        tmpa |= waveform & mask10;
        TCCRA_::write(tmpa);
        }
    };

/** Hardware timer with 2 output compare units and 3 config registers
    (TCCRA_ - TCCRC_)

  This timer unit is used by the following devices: ATMega48/88/168/328
  (Timer1)

  The TCCRA_ register contains the following configuration bits (x=#timer):

  @verbatim
  +------+------+------+------+---+---+-----+-----+
  |COMxA1|COMxA0|COMxB1|COMxB0| - | - |WGMx1|WGMx0|
  +------+------+------+------+---+---+-----+-----+
  @endverbatim

  The TCCRB_ register contains the following configuration bits (x=#timer):

  @verbatim
  +-----+-----+---+-----+-----+----+----+----+
  |ICNCx|ICESx| - |WGMx3|WGMx2|CSx2|CSx1|CSx0|
  +-----+-----+---+-----+-----+----+----+----+
  @endverbatim

  TCCRC_ register contains the following configuration bits (x=#timer):

  @verbatim
  +-----+-----+---+---+---+---+---+---+
  |FOCxA|FOCxB| - | - | - | - | - | - |
  +-----+-----+---+---+---+---+---+---+
  @endverbatim

  The TIMSK_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+---+-----+---+---+------+------+-----+
  | - | - |ICIEx| - | - |OCIExB|OCIExA|TOIEx|
  +---+---+-----+---+---+------+------+-----+
  @endverbatim

  ICIEx is only available on _Timer_2C3.

  The TIFR_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+---+----+---+---+-----+-----+----+
  | - | - |ICFx| - | - |OCFxB|OCFxA|TOVx|
  +---+---+----+---+---+-----+-----+----+
  @endverbatim

  ICFx is only available on _Timer_2C3.

*/
template <class TCNT_, class ICR_, class OCRA_, class OCRB_, class TCCRA_,
          class TCCRB_, class TCCRC_, class TIMSK_, class TIFR_>
class _Timer_2C3 : public _Timer<TCNT_, TIMSK_, 0> // 0 is TOIEx
    {
public:

    // TCCRA_
    static const byte COMxA1 = 7, COMxA0 = 6, COMxB1 = 5, COMxB0 = 4;
    static const byte WGMx1 = 1, WGMx0 = 0;

    // TCCRB_
    static const byte ICNCx = 7, ICESx = 6;
    static const byte WGMx3 = 4, WGMx2 = 3;
    static const byte CSx2 = 2, CSx1 = 1, CSx0 = 0;

    // TCCRC_
    static const byte FOCxA = 7, FOCxB = 6;

    // TIMSK_
    static const byte ICIEx = 5;
    static const byte OCIExB = 2, OCIExA = 1;
    static const byte TOIEx = 0;

    // TIFR_
    static const byte ICFx = 5;
    static const byte OCFxB = 2, OCFxA = 1;
    static const byte TOVx = 0;

    typedef ICR_ ICR;
    typedef OCRA_ OCRA;
    typedef OCRB_ OCRB;

    typedef _OutputComparator<OCRA_, TCCRA_, COMxA1, COMxA0, TIMSK_, OCIExA,
                              TIFR_, OCFxA, TCCRB_, FOCxA>
    CompA;

    typedef _OutputComparator<OCRB_, TCCRA_, COMxB1, COMxB0, TIMSK_, OCIExB,
                              TIFR_, OCFxB, TCCRB_, FOCxB>
    CompB;

    static void prescaler1() { prescaler(_BV(CSx0)); }
    static void prescaler8() { prescaler(_BV(CSx1)); }
    static void prescaler64() { prescaler(_BV(CSx1) | _BV(CSx0)); }
    static void prescaler256() { prescaler(_BV(CSx2)); }
    static void prescaler1024() { prescaler(_BV(CSx2) | _BV(CSx0)); }

    static void externalFalling() { prescaler(_BV(CSx2) | _BV(CSx1)); }
    static void externalRising()
        { prescaler(_BV(CSx2) | _BV(CSx1) | _BV(CSx0)); }

    static void modeNormal() { wgm(0); }
    static void modePhaseCorrectPWM() { wgm(_BV(WGMx0)); }
    static void modePhaseCorrectPWM9bit()
        { wgm(_BV(WGMx1) | _BV(WGMx0)); }
    static void modePhaseCorrectPWM10bit()
        { wgm(_BV(WGMx2) | _BV(WGMx0)); }
    static void modeClearTimerOnCompare() { wgm(_BV(WGMx2)); }
    static void modeFastPWM() { wgm(_BV(WGMx2) | _BV(WGMx0)); }
    static void modeFastPWM9bit() { wgm(_BV(WGMx2) | _BV(WGMx1)); }
    static void modeFastPWM10bit()
        { wgm(_BV(WGMx2) | _BV(WGMx1) | _BV(WGMx1)); }
    static void modePhaseAndFreqCorrectPWMICR()
        { wgm(_BV(WGMx3)); }
    static void modePhaseAndFreqCorrectPWMOCRA()
        { wgm(_BV(WGMx3) | _BV(WGMx0)); }
    static void modePhaseCorrectPWMICR()
        { wgm(_BV(WGMx3) | _BV(WGMx1)); }
    static void modePhaseCorrectPWMOCRA()
        { wgm(_BV(WGMx3) | _BV(WGMx1) | _BV(WGMx0)); }
    static void modeClearTimerOnCompareICR()
        { wgm(_BV(WGMx3) | _BV(WGMx2)); }
    static void modeFastPWMICR() { wgm(_BV(WGMx3) | _BV(WGMx2) | _BV(WGMx1)); }
    static void modeFastPWMOCRA()
        { wgm(_BV(WGMx3) | _BV(WGMx2) | _BV(WGMx1) | _BV(WGMx0)); }

private:
    static void prescaler(byte pre)
        {
        const byte mask = _BV(CSx2) | _BV(CSx1) | _BV(CSx0);
        byte tmp = TCCRB_::read() & ~mask;
        TCCRB_::write(tmp |= pre);
        }

    static void wgm(byte waveform)
        {
        const byte mask32 = _BV(WGMx3) | _BV(WGMx2);
        byte tmp32 = TCCRB_::read() & ~(mask32);
        tmp32 |= waveform & mask32;
        TCCRA_::write(tmp32);

        const byte mask10 = _BV(WGMx1) | _BV(WGMx0);
        byte tmp10 = TCCRA_::read() & ~mask10;
        tmp10 |= waveform & mask10;
        TCCRA_::write(tmp10);
        }
    };


/** Hardware timer with 3 output compare units and 2 config registers
    (TCCR_ and GTCCR_)

  This timer unit is used by the following devices: ATtiny25/45/85
  (Timer1)

  The TCCR_ register contains the following configuration bits (x=#timer):

  @verbatim
  +----+-----+------+------+----+----+----+----+
  |CTCx|PWMxA|COMxA1|COMxA0|CSx3|CSx2|CSx1|CSx0|
  +----+-----+------+------+----+----+----+----+
  @endverbatim

  The GTCCR_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+-----+------+------+-----+-----+----+----+
  | - |PWMxB|COMxB1|COMxB0|FOCxB|FOCxA|PSR1|PSR0|
  +---+-----+------+------+-----+-----+----+----+
  @endverbatim

  The TIMSK_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+------+------+---+---+-----+---+---+
  | - |OCIExA|OCIExB| - | - |TOIEx| - | - |
  +---+------+------+---+---+-----+---+---+
  @endverbatim

  The TIFR_ register contains the following configuration bits (x=#timer):

  @verbatim
  +---+-----+-----+---+---+---+----+---+
  | - |OCFxA|OCFxB| - | - | - |TOVx| - |
  +---+-----+-----+---+---+---+----+---+
  @endverbatim
*/
template <class TCNT_, class OCRA_, class OCRB_, class OCRC_, class TCCR_,
          class GTCCR_, class TIMSK_, class TIFR_>
class _TimerTiny_3C2 : public _Timer<TCNT_, TIMSK_, 2> // 2 is TOIEx
    {
public:

    // TCCR_
    static const byte CTCx = 7;
    static const byte PWMxA = 6;
    static const byte COMxA1 = 5, COMxA0 = 4;
    static const byte CSx3 = 3, CSx2 = 2, CSx1 = 1, CSx0 = 0;

    // GTCCR_
    static const byte PWMxB = 6;
    static const byte COMxB1 = 5, COMxB0 = 4;
    static const byte FOCxB = 3, FOCxA = 2;
    static const byte PSR1 = 1, PSR0 = 0;

    // TIMSK_
    static const byte OCIExA = 6, OCIExB = 5;
    static const byte TOIEx = 2;

    // TIFR_
    static const byte OCFxA = 6, OCFxB = 5;
    static const byte TOVx = 1;

    typedef OCRA_ OCRA;
    typedef OCRB_ OCRB;
    typedef OCRC_ OCRC;

    typedef _OutputComparator<OCRA_, TCCR_, COMxA1, COMxA0, TIMSK_, OCIExA,
                              TIFR_, OCFxA, GTCCR_, FOCxA>
    CompA;

    typedef _OutputComparator<OCRB_, TCCR_, COMxB1, COMxB0, TIMSK_, OCIExB,
                              TIFR_, OCFxB, GTCCR_, FOCxB>
    CompB;

    static void stop() { prescaler(0); }

    static void prescaler1() { prescaler(_BV(CSx0)); }
    static void prescaler2() { prescaler(_BV(CSx1)); }
    static void prescaler4() { prescaler(_BV(CSx1) | _BV(CSx0)); }
    static void prescaler8() { prescaler(_BV(CSx2)); }
    static void prescaler16() { prescaler(_BV(CSx2) | _BV(CSx0)); }
    static void prescaler32() { prescaler(_BV(CSx2) | _BV(CSx1)); }
    static void prescaler64() { prescaler(_BV(CSx2) | _BV(CSx1) | _BV(CSx0)); }
    static void prescaler128() { prescaler(_BV(CSx3)); }
    static void prescaler256() { prescaler(_BV(CSx3) | _BV(CSx0)); }
    static void prescaler512() { prescaler(_BV(CSx3) | _BV(CSx1)); }
    static void prescaler1024()
        { prescaler(_BV(CSx3) | _BV(CSx1) | _BV(CSx0)); }
    static void prescaler2048() { prescaler(_BV(CSx3) | _BV(CSx2)); }
    static void prescaler4096()
        { prescaler(_BV(CSx3) | _BV(CSx2) | _BV(CSx0)); }
    static void prescaler8192()
        { prescaler(_BV(CSx3) | _BV(CSx2) | _BV(CSx1)); }
    static void prescaler16384()
        { prescaler(_BV(CSx3) | _BV(CSx2) | _BV(CSx1) | _BV(CSx0)); }

    static void modePWMA() { TCCR_::set(PWMxA); }
    static void modePWMB() { GTCCR_::set(PWMxB); }
    static void clearOnMatchOCRC() { TCCR_::set(CTCx); }

private:
    static void prescaler(byte pre)
        {
        byte tmp = TCCR_::read() & ~15;
        TCCR_::write(tmp |= pre);
        }
    };

/*
  We are forcing gcc to inline the _Pin methods. I think this shouldn't be
  necessary, but when the _Pin methods are used via a subclass like
  _ChangeInterruptPin, gcc doesn't automatically inline these methods any more.
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
    static void modeInputTristate() __attribute__((always_inline))
        { modeInput(); clear(); }
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

template <class Pin_, class OCR_>
class _PWMPin : public Pin_
    {
public:

    /** Enable the Pin as an output and enable the noninverting PWM mode
        on the output comparator.

        Note: this does not enable PWM mode on the timer, that step needs to 
        be done separately.
     */
    static void modePWM() __attribute__((always_inline))
        {
        Pin_::modeOutput();
        OCR_::modeClear();
        }

    static void pwmOff() __attribute__((always_inline))
        {
        OCR_::disablePWM();
        }

    static void pwmWrite(byte value) __attribute__((always_inline))
        {
        OCR_::write(value);
        }
    };

// PCICR_ is the interrupt control register address, PCEN_ is the enable bit,
// PCMSK_ is pin change mask register and PCBIT_ the value bit
template <class Pin_, byte PCICR_, byte PCEN_, byte PCMSK_, byte PCBIT_>
class _ChangeInterruptPin : public Pin_
    {
public:

    static void enableChangeInterrupt() __attribute__((always_inline))
        {
        _SFR_IO8(PCICR_) |= _BV(PCEN_);
        _SFR_IO8(PCMSK_) |= _BV(PCBIT_);
        }
    static void disableChangeInterrupt() __attribute__((always_inline))
        {
        if (_SFR_IO8(PCMSK_) &= ~_BV(PCBIT_))
            _SFR_IO8(PCICR_) &= ~_BV(PCEN_);
        }
    };

#if defined (ADMUX) && defined (ADCSRA) && defined (ADSC) && defined (ADCH) \
  && defined (ADCL)

template <class Pin_, byte AIN_>
class _AnalogPin : public Pin_
    {
    static void analogStart(uint8_t reference) __attribute__((always_inline))
        {
        // set the analog reference (high two bits of ADMUX) and select the
        // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
        // to 0 (the default).
        ADMUX = (reference << 6) | (AIN_ & 0x07);
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

        t = Timer::TCNT::read();
        m = timer_overflow_count % (1 << TIMER16_MICRO_SCALE);

        if ((Timer::TIFR::read() & _BV(TOV0)) && (t == 0))
            m++;

        // FIXME: Timer::PRESCALE not actually defined yet, see CLOCK16_PRESCALE
        return ((m << 8) + t) * (Timer::PRESCALE / (F_CPU / 1000000L));
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
    static void write(Out *out, const byte *b, byte n)
        {
        for (int k = 0; k < n; ++k)
            write(out, b[k]);
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

template <class Out> class DecimalWriter
    {
public:
    static void write(Out *out, uint32_t d, char digits)
        {
        if (d == 0 && digits <= 0)
            return;
        write(out, d/10, digits - 1);
        out->write('0' + d % 10);
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
