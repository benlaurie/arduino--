#include "arduino++.h"
#include "timer16.h"
#include "pushbutton.h"

PushButton<Timer16, Arduino::D11, 20> Button;

int main(void) 
    {
    Arduino::init();
    Arduino::D13::modeOutput();

    Button.init();

    for (;;)
        {
        if (Button.read() == Button.keyup)
            {
            Arduino::D13::set();
            Timer16::delay(Button.duration());
            Arduino::D13::clear();
            }
        }
    }
    
