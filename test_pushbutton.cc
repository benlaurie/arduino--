#include "arduino++.h"
#include "clock16.h"
#include "pushbutton.h"

PushButton<Clock16, Arduino::D11, 20> Button;

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
            Clock16::delay(Button.duration());
            Arduino::D13::clear();
            }
        }
    }
    
