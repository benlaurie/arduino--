/* 
   Use a pin change interrupt to turn on a LED when a button is pressed
*/

#include "arduino--.h"
#include <avr/sleep.h>

ISR(PCINT0_vect) 
    {
    Arduino::D11::read() ? Arduino::D13::clear() : Arduino::D13::set();
    }

int main(void)
    {
    Arduino::interrupts();
    // Arduino Pin D13 is typically connected to an LED
    Arduino::D13::modeOutput();
    
    // make D11 input and activate internal pullups
    Arduino::D11::modeInputPullup();

    // Initialise the LED on D13
    Arduino::D11::read() ? Arduino::D13::clear() : Arduino::D13::set();

	// enable the pin change interrupt
    Arduino::D11::enableChangeInterrupt();

    set_sleep_mode(SLEEP_MODE_IDLE);
    while(true)
        {
        sleep_enable();
        sleep_cpu();
        }
    
    return 0;
    }
