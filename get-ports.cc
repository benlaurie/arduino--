#define _SFR_ASM_COMPAT 1
#include <avr/io.h>

#define D(port)  *x = port - __SFR_OFFSET

int main(int argc, char **argv)
    {
    volatile uint8_t *x = 0;

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) \
    || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) \
    || defined (__AVR_ATmega168P__)
#include "defs/ports_mx8.i"
#elif defined (__AVR_ATtiny85__) || defined (__AVR_ATtiny45__) \
  || defined (__AVR_ATTiny25__)
#include "defs/ports_tnx5.i"
#else
#error "No port definition found"
#endif

    return 0;
    }
