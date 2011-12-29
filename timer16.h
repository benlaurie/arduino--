#ifndef ARDUINO_MINUS_MINUS_TIMER16_H
#define ARDUINO_MINUS_MINUS_TIMER16_H

#ifdef ARDUINO_MINUS_MINUS_TIMER32_H
#error "Only one timer resolution can be used"
#endif

/* This file should be included - once - in the main C++ file of the 
   application.

   Other headers should - if at all possible - not include this file and
   use a template instead.
 */

/** This is a Timer with 16bit timer resolution.
    
    The value from Timer16::millis() will wrap around after about 65 seconds.

    The recommended way to use a timer value in user code is:

    typename Timer16::time_res_t now = Arduino16::millis();

    Motivation: With Timer16 instead of Arduino32, Lars has seen a code size 
    reduction of 238 bytes with avr-gcc 4.6.1.
 */
typedef _Timer<uint16_t> Timer16;

/* 
 * Implementation of the timer ISR for 16 bits resolution 
 */
ISR(TIMER0_OVF_vect)
    {
    Timer16::timer0_overflow_count++;
    // timer 0 prescale factor is 64 and the timer overflows at 256
    Timer16::timer0_clock_cycles += 64UL * 256UL;
    while (Timer16::timer0_clock_cycles > F_CPU / 1000L)
        {
        Timer16::timer0_clock_cycles -=  F_CPU / 1000L;
        Timer16::timer0_millis++;
        }
    }

#endif
