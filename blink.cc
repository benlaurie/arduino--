/* 
   The hello world of arduino--, a C++ take on the Arduino libraries.

   Blinks an LED.
*/

#include "arduino++.h"

int main(void)
	{
	Arduino::init();
	Arduino::D13::out();

    while(true)
		{
		Arduino::D13::toggle();
		Arduino::delay(2000);
		}
	
    return 0;
	}
