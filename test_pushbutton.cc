#include "arduino++.h"
#include "pushbutton.h"

PushButton<Arduino::D11, 20, uint16_t> Button;

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
            Arduino::delay(Button.duration());
            Arduino::D13::clear();
            }
        }
    }
    
