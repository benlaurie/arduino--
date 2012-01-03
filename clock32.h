#ifndef ARDUINO_MINUS_MINUS_CLOCK32_H
#define ARDUINO_MINUS_MINUS_CLOCK32_H

#ifdef ARDUINO_MINUS_MINUS_CLOCK16_H
#error "Only one clock resolution can be used"
#endif

/* This file should be included - once - in the main C++ file of the 
   application.

   Other headers should - if at all possible - not include this file and
   use a template instead.
 */

/** This is a clock with 32bit clock resolution.
    
    The recommended way to use a clock value in user code is:

    typename Clock32::time_res_t now = Arduino32::millis();

    The value from Clock32::millis() will wrap around after about 49 days.
 */
typedef _Clock<uint32_t> Clock32;

/* 
 * Implementation of the clock ISR for 32 bits resolution 
 */
ISR(TIMER0_OVF_vect)
    {

	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	typename Clock32::time_res_t m = Clock32::timer0_millis;
	uint16_t f = Clock32::timer0_fract;

	m += (64 * (256 / (F_CPU / 1000000))) / 1000;
	f += (64 * (256 / (F_CPU / 1000000))) % 1000;
	if (f >= 1000) {
		f -= 1000;
		m += 1;
	}

    Clock32::timer0_fract = f;
    Clock32::timer0_millis = m;
    Clock32::timer0_overflow_count++;
    }

#endif
