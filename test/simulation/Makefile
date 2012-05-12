# GNU make

OPTIMIZE = -O3 -Os
DEFS = -I /usr/local/avr/avr/include -I../.. 
LIBS = -B /usr/local/avr/avr/lib
CC = avr-gcc
CXX = avr-g++
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

CFLAGS = -g -Wall $(OPTIMIZE) $(DEFS)
CXXFLAGS = -g -Wall $(OPTIMIZE) $(DEFS)
LDFLAGS = -Wl,-Map,$@.map $(LIBS)

ELF = atmega328_digital_pins.elf atmega328_pwm_pins.elf

all: $(ELF) 

.SUFFIXES: .elf

atmega328_digital_pins.elf: atmega328_digital_pins.o
	$(CC) $(CFLAGS) -mmcu=atmega328p $(LDFLAGS) -o $@ $<

atmega328_digital_pins.o: atmega328_digital_pins.cc 
	$(CXX) $(CXXFLAGS) -mmcu=atmega328p -DF_CPU=4000000 -c -o $(<:.cc=.o) $<

atmega328_digital_pins.d: atmega328_digital_pins.cc
	$(CC) $(DEFS) -mmcu=atmega328p -MM $< > $@

atmega328_pwm_pins.elf: atmega328_pwm_pins.o
	$(CC) $(CFLAGS) -mmcu=atmega328p $(LDFLAGS) -o $@ $<

atmega328_pwm_pins.o: atmega328_pwm_pins.cc 
	$(CXX) $(CXXFLAGS) -mmcu=atmega328p -DF_CPU=4000000 -c -o $(<:.cc=.o) $<

atmega328_pwm_pins.d: atmega328_pwm_pins.cc
	$(CC) $(DEFS) -mmcu=atmega328p -MM $< > $@

.PHONY: test clean

test: all tests.py adapter.py
	python tests.py

clean:
	rm -f *.o *.elf *.d *.map *.pyc

-include $(ELF:.elf=.d)

