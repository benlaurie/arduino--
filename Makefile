MCU_TARGET = atmega168
OPTIMIZE = -O3 -Os
DEFS = -I /usr/local/avr/avr/include -DF_CPU=16000000
LIBS = -B /usr/local/avr/avr/lib
CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
CXXFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
LDFLAGS = -Wl,-Map,$@.map $(LIBS)

all: .depend test_enc28j60.bin test_enc28j60.lst

.depend: *.cc *.h
	$(CC) -MM *.cc > .depend

.SUFFIXES: .lst .elf .bin

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $(<:.cpp=.o) $<

.c.o:
	$(CC) $(CFLAGS) -c -o $(<:.c=.o) $<

.elf.bin:
	$(OBJCOPY) -j .text -j .data -O binary $< $@

.elf.lst:
	$(OBJDUMP) -h -S $< > $@

.o.elf:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

avr-ports.h: get-ports.lst extract-ports.pl
	./extract-ports.pl < get-ports.lst > avr-ports.h

clean:
	rm -f *.o *.map *.lst *.elf *.bin avr-ports.h


