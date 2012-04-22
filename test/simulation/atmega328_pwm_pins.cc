/* 
*/

#include "arduino--.h"

int main(void)
    {
    Timer1::prescaler1();
    Timer1::modePhaseCorrectPWM();

    Pin::B1::modePWM();
    Pin::B1::pwmWrite(16);

    _delay_ms(1);

    return 0;
    }
