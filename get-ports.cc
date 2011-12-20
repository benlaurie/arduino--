#define _SFR_ASM_COMPAT 1
#include <avr/io.h>

#define D(port)  *x = port - __SFR_OFFSET

int main(int argc, char **argv)
    {
    volatile uint8_t *x = 0;

    D(PORTB);
    D(DDRB);
    D(PINB);

    D(PORTC);
    D(DDRC);
    D(PINC);

    D(PORTD);
    D(DDRD);
    D(PIND);

    return 0;
    }
