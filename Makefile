include Makefile.local

OPTIMIZE = -O3 -Os
DEFS = -I /usr/local/avr/avr/include -DF_CPU=$(CPU_FREQUENCY)
LIBS = -B /usr/local/avr/avr/lib
CC = avr-gcc
CXX = avr-g++
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
CXXFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
LDFLAGS = -Wl,-Map,$@.map $(LIBS)

BIN = blink.bin test_clock.bin test_enc28j60.bin onewire_test.bin test_ip.bin \
      test_serial.bin test_rf12.bin test_nanode_mac.bin test_pushbutton.bin \
      test_watchdog.bin test_serial.bin test_rf12.bin test_nanode_mac.bin \
      test_timer.bin test_onewire_serial.bin blink_nanode.bin \
      test_rf12_layered.bin test_star.bin test_star_slave.bin

all: avr-ports.h .depend $(BIN) $(BIN:.bin=.lst) sizes/sizes.html

.depend: *.cc *.h
	$(CC) -mmcu=$(MCU_TARGET) -MM *.cc > .depend

.SUFFIXES: .lst .elf .bin _upload

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

sizes/recent_sizes.json sizes/sizes.html: $(BIN)
	python sizes/sizes.py recent generate

avr-ports.h: get-ports.lst extract-ports.pl
	./extract-ports.pl -f $(CPU_FREQUENCY) < get-ports.lst > avr-ports.h

clean:
	rm -f *.o *.map *.lst *.elf *.bin avr-ports.h .depend

.bin_upload:
	avrdude -F -V -p $(MCU_TARGET) -P $(AVR_TTY) -c $(AVR_PROGRAMMER) -b $(AVR_RATE) -U flash:w:$<

.sinclude ".depend"
