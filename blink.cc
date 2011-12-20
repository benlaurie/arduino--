/* 
   The hello world of arduino--, a C++ take on the Arduino libraries.

   Blinks an LED connected to digital pin 13 (which is connected to an LED
   on all Arduino variants that we know of).
*/

#include "arduino++.h"

int main(void)
	{
	// Arduino Pin D13 is an output
	Arduino::D13::out();

	while(true)
		{
		// toggle the pin
		Arduino::D13::toggle();
		// wait
		Arduino::constantDelay(2000);
		}
	
	return 0;
	}
