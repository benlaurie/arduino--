/* 
   The hello world of arduino--, a C++ take on the Arduino libraries.

   Blinks an LED.
*/

#include "arduino++.h"

int main(void)
{
	Arduino::init();
	Pin::B5::Out();

	while(true) {
		Pin::B5::Toggle();
		Arduino::delay(1000);
	}

	return 0;
}
