#ifndef ARDUINO_MINUS_MINUS_CLOCK16_H
#define ARDUINO_MINUS_MINUS_CLOCK16_H

#ifdef ARDUINO_MINUS_MINUS_CLOCK32_H
#error "Only one clock resolution can be used"
#endif

/* This file should be included - once - in the main C++ file of the 
   application.

   Other headers should - if at all possible - not include this file and
   use a template instead.
 */

/** This is a Clock with 16bit clock resolution.
    
    The value from Clock16::millis() will wrap around after about 65 seconds.

    The recommended way to use a clock value in user code is:

    typename Clock16::time_res_t now = Arduino16::millis();

    Motivation: With Clock16 instead of Clock32, Lars has seen a code size 
    reduction of 238 bytes with avr-gcc 4.6.1.
 */
typedef _Clock<uint16_t> Clock16;

/* 
 * Implementation of the clock ISR for 16 bits resolution 
 */
ISR(TIMER0_OVF_vect)
    {

	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	typename Clock16::time_res_t m = Clock16::timer0_millis;
	uint16_t f = Clock16::timer0_fract;

	m += (64 * (256 / (F_CPU / 1000000))) / 1000;
	f += (64 * (256 / (F_CPU / 1000000))) % 1000;
	if (f >= 1000) {
		f -= 1000;
		m += 1;
	}

    Clock16::timer0_fract = f;
    Clock16::timer0_millis = m;
    Clock16::timer0_overflow_count++;
    }

#endif
