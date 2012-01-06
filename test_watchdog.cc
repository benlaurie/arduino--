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
    Serial.write("Boing!\r\n");
    wdt_enable(WDTO_2S);
    wdt_reset();
    for ( ; ; )
	;
    }
