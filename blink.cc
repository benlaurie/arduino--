/* 
   The hello world of arduino--, a C++ take on the Arduino libraries.

   Blinks an LED connected to digital pin 13 (which is connected to an LED
   on all Arduino variants that we know of).
*/

#include "arduino++.h"
#include <avr/sleep.h>

int main(void)
    {
    // Arduino Pin D13 is an output
    Arduino16::D13::modeOutput();

    while(true)
        {
        // toggle the pin
        Arduino16::D13::toggle();
        // wait
        _delay_ms(2000);
        }
    
    return 0;
    }
