#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

uint16_t micros(int freq, int overflow, uint8_t ticks)
    {
    return ((overflow << 8) + ticks) * (64 / freq);
    }

int main(int argc, char * argv[])
    {
    if (argc < 2)
        {
        printf("usage: ./test_micros freq(MHz) [overflow_count] [ticks]\n");
        exit(2);
        }

    int freq = atoi(argv[1]);
    uint8_t scale = sizeof(uint16_t) * 8 
        - ceil(log(64.0 / freq * (1 << 8))/log(2.0));

    printf("CPU freq: %d MHz, scale: %d, resolution: %d\n", freq, scale,
      64/freq);
    printf("maximum microsecond value: %u\n", 
           micros(freq, (1 << scale) - 1, 255));
    printf("maximum overflow count: %u\n", (1 << scale) - 1);

    if (argc > 3)
        {
        uint16_t m = atoi(argv[2]);
        uint16_t t = atoi(argv[3]);

        if (t == 256)
            ++m;
        else if (t > 256)
            {
            printf("error: ticks may be at most 256");
            exit(2);
            }

        m %= (1 << scale);

        printf("micros: %u\n", micros(freq, m, t));
        }
    }
