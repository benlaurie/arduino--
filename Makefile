# GNU make (hint for sizes.py, must be in the first line)

include Makefile.local

OPTIMIZE = -O3 # -Os
DEFS = -I /usr/local/avr/avr/include -I. -DF_CPU=$(CPU_FREQUENCY)
LIBS = -B /usr/local/avr/avr/lib
CC = avr-gcc
CXX = avr-g++
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

VPATH=test

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
CXXFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
LDFLAGS = -Wl,-Map,$@.map $(LIBS)

BIN = test/blink.bin test/test_clock.bin test/test_enc28j60.bin \
      test/test_onewire.bin test/test_ip.bin test/test_serial.bin \
      test/test_rf12.bin test/test_nanode_mac.bin test/test_pushbutton.bin \
      test/test_watchdog.bin test/test_serial.bin test/test_pcint.bin \
      test/test_rf12.bin test/test_nanode_mac.bin test/test_timer.bin \
      test/test_onewire_serial.bin test/blink_nanode.bin \
      test/test_rf12_layered.bin test/test_star.bin test/test_star_slave.bin \
      test/test_star_slave_onewire.bin test/test_star_bridge.bin \
      test/test_ip_layered.bin test/test_clock_serial.bin \
      test/test_clock_nanode.bin test/test_ws2811.bin \
      live/star_slave_onewire.bin

all: avr-ports.h $(BIN) $(BIN:.bin=.lst) sizes/sizes.html

.depend: avr-ports.h *.cc *.h test/*.cc live/*.cc
	$(CC) $(DEFS) -mmcu=$(MCU_TARGET) -MM *.cc > .depend
	$(CC) $(DEFS) -mmcu=$(MCU_TARGET) -MM test/*.cc | sed 's;^\(.*\):;test/\1:;' >> .depend
	$(CC) $(DEFS) -mmcu=$(MCU_TARGET) -MM live/*.cc | sed 's;^\(.*\):;live/\1:;' >> .depend

.SUFFIXES: .elf .lst .bin _upload

.cc.o: 
	$(CXX) $(CXXFLAGS) -c -o $(<:.cc=.o) $<

.c.o:
	$(CC) $(CFLAGS) -c -o $(<:.c=.o) $<

.elf.bin:
	$(OBJCOPY) -j .text -j .data -O binary $< $@

.elf.lst:
	$(OBJDUMP) -h -S $< > $@

.o.elf:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

test/test.grb: test/make_grb.py
	python test/make_grb.py > test/test.grb

sizes/recent_sizes.json sizes/sizes.html: $(BIN)
	python sizes/sizes.py recent generate

get-ports.cc: Makefile.local

avr-ports.h: get-ports.lst extract-ports.pl
	./extract-ports.pl -f $(CPU_FREQUENCY) < get-ports.lst > avr-ports.h

history:
	python sizes.py history generate

sizeclean:
	rm -f sizes/recent_sizes.json

clean: sizeclean
	rm -f *.o *.map *.lst *.elf *.bin test/*.o test/*.map test/*.lst \
	test/*.elf test/*.bin avr-ports.h .depend 

.bin_upload:
	avrdude -F -V -p $(MCU_TARGET) -P $(AVR_TTY) -c $(AVR_PROGRAMMER) -b $(AVR_RATE) -U flash:w:$<

-include .depend
