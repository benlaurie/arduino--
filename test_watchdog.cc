#include "arduino++.h"
#include "serial.h"
#include <avr/wdt.h>

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

void get_mcusr(void) 
  __attribute__((naked))
  __attribute__((section(".init3")));
void get_mcusr(void)
    {
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
    }

// This causes a hard hang as soon as the watchdog resets the CPU.

int main()
    {
    Arduino::init();
    Serial.begin(57600);
    Serial.write("Boing!\r\n");
    wdt_enable(WDTO_2S);
    for (byte n = 0; n < 30; ++n)
	{
	_delay_ms(500);
	Serial.write("gnioB");
	Serial.writeHex(n);
	Serial.write("\r\n");
	wdt_reset();
	}
    for ( ; ; )
	;
    }
