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

typedef _Timer_2C2<_Register<NTCNT0>, _Register<NOCR0A>, _Register<NOCR0B>, 
                   _Register<NTCCR0A>, _Register<NTCCR0B>, _Register<NTIFR>, 
                   _Register<NTIMSK> > 
Timer0;

typedef _TimerTiny_3C2<_Register<NTCNT1>, _Register<NOCR1A>, _Register<NOCR1B>,
                       _Register<NOCR1C>, _Register<NTCCR>, _Register<NGTCCR>,
                       _Register<NTIFR>, _Register<NTIMSK> >
Timer1;

class Pin
    {
public:
    typedef _Pin<NDDRB, NPORTB, NPINB, PB0> D_B0;
    typedef _PWMPin<D_B0, Timer0::CompA> OC0A;
    typedef OC0A B0;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB1> D_B1;
    typedef _PWMPin<D_B1, Timer0::CompB> OC0B;
    typedef _PWMPin<D_B1, Timer1::CompA> OC1A;
    typedef OC0B B1;

    // The ADC ordering is irregular
    typedef _Pin<NDDRB, NPORTB, NPINB, PB2> D_B2;
    typedef _AnalogPin<D_B2, 1> ADC1;
    typedef _ChangeInterruptPin<ADC1, NPCMSK, PCIE, NGIFR, PCIF> B2;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB3> D_B3;
    typedef _AnalogPin<D_B3, 3> ADC3;
    typedef _ChangeInterruptPin<ADC3, NPCMSK, PCIE, NGIFR, PCIF> B3;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB4> D_B4;
    typedef _AnalogPin<D_B4, 2> ADC2;
    typedef _PWMPin<ADC2, Timer1::CompB> OC1B;
    typedef _ChangeInterruptPin<OC1B, NPCMSK, PCIE, NGIFR, PCIF> B4;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB5> D_B5;
    typedef _AnalogPin<D_B5, 0> ADC0;
    typedef _ChangeInterruptPin<ADC0, NPCMSK, PCIE, NGIFR, PCIF> B5;
    };

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
        ///Timer1::prescaler64();
        // put timer 1 in 8-bit phase correct pwm mode
        //Timer1::phaseCorrectPWM();

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
