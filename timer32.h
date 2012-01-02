#ifndef ARDUINO_MINUS_MINUS_TIMER32_H
#define ARDUINO_MINUS_MINUS_TIMER32_H

#ifdef ARDUINO_MINUS_MINUS_TIMER16_H
#error "Only one timer resolution can be used"
#endif

/* This file should be included - once - in the main C++ file of the 
   application.

   Other headers should - if at all possible - not include this file and
   use a template instead.
 */

/** This is a Timer with 32bit timer resolution.
    
    The recommended way to use a timer value in user code is:

    typename Timer32::time_res_t now = Arduino32::millis();

    The value from Timer32::millis() will wrap around after about 49 days.
 */
typedef _Timer<uint32_t> Timer32;

/* 
 * Implementation of the timer ISR for 32 bits resolution 
 */
ISR(TIMER0_OVF_vect)
    {

	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	typename Timer32::time_res_t m = Timer32::timer0_millis;
	uint16_t f = Timer32::timer0_fract;

	m += (64 * (256 / (F_CPU / 1000000))) / 1000;
	f += (64 * (256 / (F_CPU / 1000000))) % 1000;
	if (f >= 1000) {
		f -= 1000;
		m += 1;
	}

    Timer32::timer0_fract = f;
    Timer32::timer0_millis = m;
    Timer32::timer0_overflow_count++;
    }

#endif
