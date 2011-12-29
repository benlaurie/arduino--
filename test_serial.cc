#include "arduino++.h"
#include "serial.h"

int main()
    {
    // Without this it sends but does not receive.
    Arduino::init();
    Serial.begin(57600);
    char c = 'x';
    for( ; ; )
	{
	Serial.write(c);
	_delay_ms(500);
	int t = Serial.read();
	if (t >= 0)
	    c = t;
	}
    }
