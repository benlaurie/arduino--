#ifndef ARDUINO_MINUS_MINUS_TNX5
#define ARDUINO_MINUS_MINUS_TNX5

// AREF, Internal Vref turned off 
const uint8_t REF_VCC = 0;
// AVCC with external capacitor at AREF pin
const uint8_t REF_AREF_EXT = 1;
// Internal 1.1V Voltage Reference
const uint8_t REF_INT_1_1 = 2;
// Internal 2.56V Voltage Reference, AREF/PB0 disconnected
const uint8_t REF_INT_2_56 = 6;
// Internal 2.56V Voltage Reference with bypass capacitor at AREF/PB0
const uint8_t REF_INT_2_56_EXT_CAP = 7;

class Pin
    {
public:
    typedef _Pin<NDDRB, NPORTB, NPINB, PB0> B0;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB1> B1;
    typedef _AnalogPin< NDDRB, NPORTB, NPINB, PB2> B2;
    typedef _AnalogPin<NDDRB, NPORTB, NPINB, PB3> B3;
    typedef _AnalogPin<NDDRB, NPORTB, NPINB, PB4> B4;
    typedef _AnalogPin<NDDRB, NPORTB, NPINB, PB5> B5;
    };

typedef _Timer<_Register<NTCNT0>, _Register<NOCR0A>, _Register<NOCR0B>, 
               _Register<NTCCR0A>, _Register<NTCCR0B>, _Register<NTIFR>, 
               _Register<NTIMSK>, TOIE0, OCIE0A, OCF0A, OCIE0B, OCF0B,
               CS00, CS01, CS02, WGM00, WGM01, WGM02> Timer0;

class Arduino : public AVRBase
    {
public:

    static void init()
        {
        interrupts();
    
        Timer0::fastPWM();

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

        // set a2d prescale factor to 128
        // 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
        // XXX: this will not work properly for other clock speeds, and
        // this code should use F_CPU to determine the prescale factor.
        _SFR_BYTE(ADCSRB) |= (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0));

        // enable a2d conversions
        _SFR_BYTE(ADCSRB) |= _BV(ADEN);
        }
    };

#endif // ARDUINO_MINUS_MINUS_TNX5
