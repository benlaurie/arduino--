#include "arduino++.h"

int main(void)
    {
    Arduino::D13::modeOutput();
    while(true)
        {
	Arduino::D13::toggle();
        _delay_ms(200);
        }
    return 0;
    }
