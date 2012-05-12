/* 
*/

#include "arduino--.h"

int main(void)
    {
    // Timer0
    Timer0::prescaler1();
    Timer0::modeFastPWM();

    Pin::D5::modePWM();
    Pin::D5::pwmWrite(4);

    Pin::D6::modePWM();
    Pin::D6::pwmWrite(8);

    // Timer1
    Timer1::prescaler1();
    Timer1::modePhaseCorrectPWM();

    Pin::B1::modePWM();
    Pin::B1::pwmWrite(16);

    Pin::B2::modePWM();
    Pin::B2::pwmWrite(32);

    // Timer2
    Timer2::prescaler8();
    Timer2::modePhaseCorrectPWM();

    Pin::B3::modePWM();
    Pin::B3::pwmWrite(64);

    Pin::D3::modePWM();
    Pin::D3::pwmWrite(128);

    _delay_ms(2);

    return 0;
    }
