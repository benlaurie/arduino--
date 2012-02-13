#define _SFR_ASM_COMPAT 1
#include <avr/io.h>

#define D(port)  *x = port - __SFR_OFFSET

int main(int argc, char **argv)
    {
    volatile uint8_t *x = 0;

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#include "defs/ports_m328.i"
#elif defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__)
#include "defs/ports_m168.i"
#endif

    return 0;
    }
