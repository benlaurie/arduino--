#ifndef ARDUINO_MINUS_MINUS_MX8
#define ARDUINO_MINUS_MINUS_MX8

// Analog input voltage references

// AREF, Internal Vref turned off 
const uint8_t REF_AREF = 0;
// AVCC with external capacitor at AREF pin
const uint8_t REF_AVCC_EXT_CAP = 1;
// Internal 1.1V Voltage Reference with external capacitor at AREF pin
const uint8_t REF_INT_1_1_EXT_CAP = 3;

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

class Pin
    {
public:
    // Port B
    typedef _Pin<NDDRB, NPORTB, NPINB, PB0> D_B0;
    typedef _ChangeInterruptPin<D_B0, NPCMSK0, NPCICR, PCIE0, PCINT0> B0; 

    typedef _Pin<NDDRB, NPORTB, NPINB, PB1> D_B1;
    typedef _ChangeInterruptPin<D_B1, NPCMSK0, NPCICR, PCIE0, PCINT1> B1;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB2> D_B2;
    typedef _ChangeInterruptPin<D_B2, NPCMSK0, NPCICR, PCIE0, PCINT2> B2;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB3> D_B3;
    typedef _ChangeInterruptPin<D_B3, NPCMSK0, NPCICR, PCIE0, PCINT3> B3;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB4> D_B4;
    typedef _ChangeInterruptPin<D_B4, NPCMSK0, NPCICR, PCIE0, PCINT4> B4;

    typedef _Pin<NDDRB, NPORTB, NPINB, PB3> D_B5;
    typedef _ChangeInterruptPin<D_B5, NPCMSK0, NPCICR, PCIE0, PCINT5> B5;

    // Port C
    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC0, 0> ADC0;
    typedef _ChangeInterruptPin<ADC0, NPCMSK1, NPCICR, PCIE1, PCINT8> C0;

    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC1, 1> ADC1;
    typedef _ChangeInterruptPin<ADC1, NPCMSK1, NPCICR, PCIE1, PCINT9> C1;

    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC2, 2> ADC2;
    typedef _ChangeInterruptPin<ADC2, NPCMSK1, NPCICR, PCIE1, PCINT10> C2;

    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC3, 3> ADC3;
    typedef _ChangeInterruptPin<ADC3, NPCMSK1, NPCICR, PCIE1, PCINT11> C3;

    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC4, 4> ADC4;
    typedef _ChangeInterruptPin<ADC4, NPCMSK1, NPCICR, PCIE1, PCINT12> C4;

    // Port D
    typedef _Pin<NDDRD, NPORTD, NPIND, PD0> D_D0;
    typedef _ChangeInterruptPin<D_D0, NPCMSK2, NPCICR, PCIE2, PCINT16> D0;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD1> D_D1;
    typedef _ChangeInterruptPin<D_D1, NPCMSK2, NPCICR, PCIE2, PCINT17> D1;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD2> D_D2;
    typedef _ChangeInterruptPin<D_D2, NPCMSK2, NPCICR, PCIE2, PCINT18> D2;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD3> D_D3;
    typedef _ChangeInterruptPin<D_D3, NPCMSK2, NPCICR, PCIE2, PCINT19> D3;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD4> D_D4;
    typedef _ChangeInterruptPin<D_D4, NPCMSK2, NPCICR, PCIE2, PCINT20> D4;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD5> D_D5;
    typedef _ChangeInterruptPin<D_D5, NPCMSK2, NPCICR, PCIE2, PCINT21> D5;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD6> D_D6;
    typedef _ChangeInterruptPin<D_D6, NPCMSK2, NPCICR, PCIE2, PCINT22> D6;

    typedef _Pin<NDDRD, NPORTD, NPIND, PD7> D_D7;
    typedef _ChangeInterruptPin<D_D7, NPCMSK2, NPCICR, PCIE2, PCINT23> D7;

    typedef Pin::B2 SPI_SS;
    typedef Pin::B3 SPI_MOSI;
    typedef Pin::B4 SPI_MISO;
    typedef Pin::B5 SPI_SCK;
    };

typedef _Timer<_Register<NTCNT0>, _Register<NOCR0A>, _Register<NOCR0B>, 
               _Register<NTCCR0A>, _Register<NTCCR0B>, _Register<NTIFR0>, 
               _Register<NTIMSK0>, TOIE0, OCIE0A, OCF0A, OCIE0B, OCF0B,
               CS00, CS01, CS02, WGM00, WGM01, WGM02> 
Timer0;

typedef _Timer<_Register16<NTCNT1>, _Register16<NOCR1A>, _Register16<NOCR1B>,
               _Register<NTCCR1A>, _Register<NTCCR1B>, _Register<NTIFR1>, 
               _Register<NTIMSK1>, TOIE1, OCIE1A, OCF1A, OCIE1B, OCF1B,
               CS10, CS11, CS12, WGM10, WGM11, WGM12> 
Timer1;

typedef _Timer<_Register<NTCNT2>, _Register<NOCR2A>, _Register<NOCR2B>,
               _Register<NTCCR2A>, _Register<NTCCR2B>, _Register<NTIFR2>, 
               _Register<NTIMSK2>, TOIE2, OCIE2A, OCF2A, OCIE2B, OCF2B, 
               CS20, CS21, CS22, WGM20, WGM21, WGM22> 
Timer2;

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

        // set timer 2 prescale factor to 64
        Timer2::prescaler64();
        Timer2::phaseCorrectPWM();

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
        UCSR0B = 0;
        }
    };

#endif // ARDUINO_MINUS_MINUS_MX8
