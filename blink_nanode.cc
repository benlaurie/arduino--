/* 
   The hello world of arduino--, a C++ take on the Arduino libraries.

   Blinks an LED connected to digital pin 13 (which is connected to an LED
   on all Arduino variants that we know of).
*/

#include "arduino++.h"
#include <avr/sleep.h>

int main(void)
    {
    Arduino::D6::modeOutput();

    while(true)
        {
        Arduino::D6::toggle();
        _delay_ms(2000);
        }
    
    return 0;
    }
