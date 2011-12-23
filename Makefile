OPTIMIZE = -O3
DEFS = -I /usr/local/avr/avr/include -DF_CPU=16000000
LIBS = -B /usr/local/avr/avr/lib
CC = avr-gcc
CXX = avr-g++
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
CXXFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
LDFLAGS = -Wl,-Map,$@.map $(LIBS)

.include "Makefile.local"

# Define all object files.
OBJ = arduino++.o

all: avr-ports.h .depend blink.bin blink.lst blink2.bin blink2.lst \
     test_enc28j60.bin test_enc28j60.lst onewire_test.bin onewire_test.lst \
     libarduino++.a 

.depend: *.cc *.h
	$(CC) -MM *.cc > .depend

libarduino++.a: $(OBJ)
	@for i in $(OBJ); do echo $(AR) $@ rcs $$i; $(AR) rcs $@ $$i; done

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
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< libarduino++.a

avr-ports.h: get-ports.lst extract-ports.pl
	./extract-ports.pl < get-ports.lst > avr-ports.h

clean:
	rm -f *.o *.map *.lst *.elf *.bin avr-ports.h .depend libarduino.a

upload: all
	avrdude -F -V -p atmega328p -P /dev/tty.usbserial-FTDOMH20 -c arduino  -b 115200 -U flash:w:blink.bin
