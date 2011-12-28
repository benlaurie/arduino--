#include "arduino++.h"
#include "arduino++timer16.h"
#include "pushbutton.h"

PushButton<Arduino16, Arduino16::D11, 20> Button;

int main(void) 
    {
    Arduino16::init();
    Arduino16::D13::modeOutput();

    Button.init();

    for (;;)
        {
        if (Button.read() == Button.keyup)
            {
            Arduino16::D13::set();
            Arduino16::delay(Button.duration());
            Arduino16::D13::clear();
            }
        }
    }
    
