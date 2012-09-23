#include "arduino--.h"
#include "clock16.h"
#include "serial.h"

int main(void)
    {
    Arduino::init();
    Serial.begin(57600);
    Serial.write("Testing clock\r\n");
    while(true)
        {
	Serial.writeDecimal(Clock16::millis());
	Serial.write("\r\n");
	_delay_ms(500);
        }
    return 0;
    }
