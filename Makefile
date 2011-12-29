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

include Makefile.local

all: avr-ports.h .depend blink.bin blink.lst blink2.bin blink2.lst \
     test_enc28j60.bin test_enc28j60.lst onewire_test.bin onewire_test.lst \
     test_ip.bin test_ip.lst test_serial.bin test_serial.lst test_rf12.bin \
     test_rf12.lst test_pushbutton.bin test_pushbutton.lst \
     libarduino++.a

.depend: *.cc *.h
	$(CC) -mmcu=$(MCU_TARGET) -MM *.cc > .depend

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
	rm -f *.o *.map *.lst *.elf *.bin avr-ports.h .depend

blink: all
	avrdude -F -V -p $(MCU_TARGET) -P $(AVR_TTY) -c $(AVR_PROGRAMMER) -b $(AVR_RATE) -U flash:w:blink.bin

pcint: all
	avrdude -F -V -p $(MCU_TARGET) -P $(AVR_TTY) -c $(AVR_PROGRAMMER) -b $(AVR_RATE) -U flash:w:pcint.bin

test_pushbutton: all
	avrdude -F -V -p $(MCU_TARGET) -P $(AVR_TTY) -c $(AVR_PROGRAMMER) -b $(AVR_RATE) -U flash:w:test_pushbutton.bin

.sinclude ".depend"
