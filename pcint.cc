/* 
   Use a pin change interrupt to turn on a LED when a button is pressed
*/

#include "arduino++.h"
#include <avr/sleep.h>

ISR(PCINT0_vect) 
    {
    Arduino16::D11::read() ? Arduino16::D13::clear() : Arduino16::D13::set();
    }

int main(void)
    {
    Arduino16::interrupts();
    // Arduino Pin D13 is typically connected to an LED
    Arduino16::D13::modeOutput();
    
    // make D11 input and activate internal pullups
    Arduino16::D11::modeInputPullup();

    // Initialise the LED on D13
    Arduino16::D11::read() ? Arduino16::D13::clear() : Arduino16::D13::set();

	// enable the pin change interrupt
    Arduino16::D11::enablePCInterrupt();

    set_sleep_mode(SLEEP_MODE_IDLE);
    while(true)
        {
        sleep_enable();
        sleep_cpu();
        }
    
    return 0;
    }
