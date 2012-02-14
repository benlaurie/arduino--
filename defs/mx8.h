#ifndef ARDUINO_MINUS_MINUS_MX8
#define ARDUINO_MINUS_MINUS_MX8

class Pin
    {
public:
    typedef _Pin<NDDRB, NPORTB, NPINB, PB0, NPCMSK0, PCINT0, PCIE0> B0;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB1, NPCMSK0, PCINT1, PCIE0> B1;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB2, NPCMSK0, PCINT2, PCIE0> B2;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB3, NPCMSK0, PCINT3, PCIE0> B3;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB4, NPCMSK0, PCINT4, PCIE0> B4;
    typedef _Pin<NDDRB, NPORTB, NPINB, PB5, NPCMSK0, PCINT5, PCIE0> B5;
        
    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC0, NPCMSK1, PCINT8, PCIE1> C0;
    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC1, NPCMSK1, PCINT9, PCIE1> C1;
    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC2, NPCMSK1, PCINT10, PCIE1> C2;
    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC3, NPCMSK1, PCINT11, PCIE1> C3;
    typedef _AnalogPin<NDDRC, NPORTC, NPINC, PC4, NPCMSK1, PCINT12, PCIE1> C4;

    typedef C0 ADC0;
    typedef C1 ADC1;
    typedef C2 ADC2;
    typedef C3 ADC3;
    typedef C4 ADC4;

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

typedef class _Interrupt<ISC00, INT0> Interrupt0;
typedef class _Interrupt<ISC10, INT1> Interrupt1;

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
