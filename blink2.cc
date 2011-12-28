#include "arduino++.h"

int main(void)
    {
    Arduino16::D13::modeOutput();
    while(true)
        {
	Arduino16::D13::toggle();
        _delay_ms(200);
        }
    return 0;
    }
