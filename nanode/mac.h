// from https://github.com/thiseldo/NanodeMAC.git

// Define NANODEMAC_SLOW to get original timings. In theory can do bit
// times of 10 us, but I can't be bothered to get the timings tight
// enough right now.

// Note that this device has at least 1.5kB of usable EEPROM if we want...

// Ben Laurie <ben@links.org> 2 Jan 2012

/***********************************************************************
 * NanodeMAC
 * Rufus Cable, June 2011 (threebytesfull)
 *
 * Library version created by Andrew Lindsay for use with Nanode and 
 * EtherShield Library at https://github.com/thiseldo/EtherShield
 *
 * Based on sample code to read the MAC address from the 11AA02E48 on the
 * back of the Nanode V5 board.
 *
 * This code is hacky and basic - it doesn't check for bus errors
 * and will probably fail horribly if it's interrupted. It's best
 * run in setup() - fetch the MAC address once and keep it. After
 * the address is fetched, it puts the chip back in standby mode
 * in which it apparently only consumes 1uA.
 *
 * Feel free to reuse this code - suggestions for improvement are
 * welcome! :)
 *
 * BITS    7   6   5   4   3   2   1   0
 * PORTD = D7  D6  D5  D4  D3  D2  D1  D0
 * PORTB = -   -   D13 D12 D11 D10 D9  D8
 *
 * Nanode has UNI/O SCIO on DIG7
 *
 ***********************************************************************/

#include "../arduino++.h"

class NanodeMAC
    {
private:
    static const uint16_t UNIO_TSTBY_US = 600;
    static const byte UNIO_THDR_US = 6;
#ifndef NANODEMAC_SLOW
    static const double QUARTER_BIT = 5;
    static const byte HALF_BIT = 10;
#else
    static const double QUARTER_BIT = 10;
    static const byte HALF_BIT = 20;
#endif

    typedef ::Pin::D7 Pin;

    void fastStandby()
	{
	Pin::set();
	Pin::modeOutput();
	}
    void standby()
	{
	fastStandby();
	AVRBase::constantDelayMicroseconds(UNIO_TSTBY_US);
	}
    void startHeader()
	{
	Pin::clear();
	AVRBase::constantDelayMicroseconds(UNIO_THDR_US);
	//sendByte(B01010101);
	sendByte(0x55);
	}
    void waitQuarterBit() { AVRBase::constantDelayMicroseconds(QUARTER_BIT); }
    void waitHalfBit() { AVRBase::constantDelayMicroseconds(HALF_BIT); }
    void bit0()
	{
	Pin::set();
	waitHalfBit();
	Pin::clear();
	waitHalfBit();
	}
    void bit1()
	{
	Pin::clear();
	waitHalfBit();
	Pin::set();
	waitHalfBit();
	}
    void sendByte(byte data)
	{
	Pin::modeOutput();
	for (int i = 0; i < 8; i++)
	    {
	    if (data & 0x80)
		bit1();
	    else
		bit0();
	    data <<= 1;
	    }
	// MAK
	bit1();
	// SAK?
	//bool sak = unio_readBit();
	readBit();
	}
    void readBytes(byte *addr, int length)
	{
	for (int i = 0; i < length; i++)
	    {
	    byte data = 0;
	    for (int b=0; b<8; b++)
		data = (data << 1) | (readBit() ? 1 : 0);
	    Pin::modeOutput();
	    if (i == length-1)
		bit0(); // NoMAK
	    else
		bit1(); // MAK
	    //bool sak = unio_readBit();
	    readBit();
	    addr[i] = data;
	    }
	}
    bool readBit()
	{
	Pin::modeInput();
	waitQuarterBit();
	bool value1 = Pin::read();
	waitHalfBit();
	bool value2 = Pin::read();
	waitQuarterBit();
	return value2 && !value1;
	}
    static byte macaddr_[6];

public:
    // Note that this can be run before the chip is initialised, so a
    // static constructor should be fine (and gets run nice and
    // early).
    NanodeMAC()
	{
	// Turn off Interrupts while we read the mac address
	AVRBase::noInterrupts();

	standby();
	startHeader();
	// address A0
	sendByte(0xA0);
	// 0x3 READ
	sendByte(0x03);
	// word address MSB 0x00
	sendByte(0x00);
	// word address LSB 0xFA
	sendByte(0xFA);
  
	// read 6 bytes into array
	readBytes(macaddr_, 6);

	// No need to wait here, since we standby() before doing anything.
	fastStandby();
  
	// Re-enable interrupts
	AVRBase::interrupts();
	}
    operator const byte *() { return macaddr_; }
    };

byte NanodeMAC::macaddr_[6];

