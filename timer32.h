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
typedef _Arduino<uint32_t> Arduino32;

/* 
 * Implementation of the timer ISR for 32 bits resolution 
 */
ISR(TIMER0_OVF_vect)
    {
    Arduino32::timer0_overflow_count++;
    // timer 0 prescale factor is 64 and the timer overflows at 256
    Arduino32::timer0_clock_cycles += 64UL * 256UL;
    while (Arduino32::timer0_clock_cycles > F_CPU / 1000L)
        {
        Arduino32::timer0_clock_cycles -=  F_CPU / 1000L;
        Arduino32::timer0_millis++;
        }
    }

#endif
