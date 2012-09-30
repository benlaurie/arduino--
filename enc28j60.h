// Based on the this:
/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 * http://www.gnu.org/licenses/gpl.html
 *
 * Based on the enc28j60.c file from the AVRlib library by Pascal Stang.
 * For AVRlib See http://www.procyonengineering.com/
 * Used with explicit permission of Pascal Stang.
 *
 * Title: Microchip ENC28J60 Ethernet Interface Driver
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/

#include "arduino--.h"
#include "spi.h"
#include <util/delay.h>

template <class CSPin> class ENC28J60
    {
    static void Select() { CSPin::clear(); }
    static void Deselect() { CSPin::set(); }
public:
    static byte ReadOp(byte op, byte address)
	{
	ScopedInterruptDisable dis;

	//CSACTIVE;
	Select();
	// issue read command
	SPDR = op | (address & ADDR_MASK);
	//waitspi();
	SPI::wait();
	// read data
	SPDR = 0x00;
	//waitspi();
	SPI::wait();
	// do dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & 0x80)
	    {
	    SPDR = 0x00;
	    //waitspi();
	    SPI::wait();
	    }
	// release CS
	//CSPASSIVE;
	Deselect();
	return SPDR;
	}
    static void WriteOp(uint8_t op, uint8_t address, uint8_t data)
	{
	ScopedInterruptDisable dis;

	//CSACTIVE;
	Select();
	// issue write command
	SPDR = op | (address & ADDR_MASK);
	//waitspi();
	SPI::wait();
	// write data
	SPDR = data;
	//waitspi();
	SPI::wait();
	//CSPASSIVE;
	Deselect();
	}
    static void ReadBuffer(uint16_t len, uint8_t* data)
	{
	ScopedInterruptDisable dis;

	//CSACTIVE;
	Select();
	// issue read command
	SPDR = ENC28J60_READ_BUF_MEM;
	//waitspi();
	SPI::wait();
	while(len)
	    {
	    len--;
	    // read data
	    SPDR = 0x00;
	    //waitspi();
	    SPI::wait();
	    *data = SPDR;
	    data++;
	    }
	// FIXME: buffer overrun!!
	*data='\0';
	//CSPASSIVE;
	Deselect();
	}
    static void WriteBuffer(uint16_t len, uint8_t* data)
	{
	ScopedInterruptDisable dis;

	//CSACTIVE;
	Select();
	// issue write command
	SPDR = ENC28J60_WRITE_BUF_MEM;
	//waitspi();
	SPI::wait();
	while(len)
	    {
	    len--;
	    // write data
	    SPDR = *data;
	    data++;
	    //waitspi();
	    SPI::wait();
	    }
	//CSPASSIVE;
	Deselect();
	}
    static void SetBank(uint8_t address)
	{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	    {
	    // set the bank
	    WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1,
			    (ECON1_BSEL1|ECON1_BSEL0));
	    WriteOp(ENC28J60_BIT_FIELD_SET, ECON1,
			    (address & BANK_MASK)>>5);
	    Enc28j60Bank = (address & BANK_MASK);
	    }
	}
    static uint8_t Read(uint8_t address)
	{
	// set the bank
	SetBank(address);
	// do the read
	return ReadOp(ENC28J60_READ_CTRL_REG, address);
	}
    static void Write(uint8_t address, uint8_t data)
	{
	// set the bank
	SetBank(address);
	// do the write
	WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
	}
    static void PhyWrite(uint8_t address, uint16_t data)
	{
	// set the PHY register address
	Write(MIREGADR, address);
	// write the PHY data
	Write(MIWRL, data);
	Write(MIWRH, data>>8);
	// wait until the PHY write completes
	while(Read(MISTAT) & MISTAT_BUSY)
	    // FIXME: why delay at all?
	    _delay_us(15);
	}
    static void clkout(uint8_t clk)
	{
	//setup clkout: 2 is 12.5MHz:
	Write(ECOCON, clk & 0x7);
	}
    static void Init(const uint8_t *macaddr)
	{
	// initialize I/O
	//CSPASSIVE; // ss=0
	Deselect();
	// ss as output:
	//pinMode(ENC28J60_CONTROL_CS, OUTPUT);
	CSPin::modeOutput();

	//pinMode(SPI_MOSI, OUTPUT);
	Pin::SPI_MOSI::modeOutput();
	//pinMode(SPI_SCK, OUTPUT);
	Pin::SPI_SCK::modeOutput();
	//pinMode(SPI_MISO, INPUT);
	Pin::SPI_MISO::modeInput();
	
	//digitalWrite(SPI_MOSI, LOW);
	Pin::SPI_MOSI::clear();
	
	//digitalWrite(SPI_SCK, LOW);
	Pin::SPI_SCK::clear();
	
	/*DDRB  |= 1<<PB3 | 1<<PB5; // mosi, sck output
	  cbi(DDRB,PINB4); // MISO is input
	  //
	  cbi(PORTB,PB3); // MOSI low
	  cbi(PORTB,PB5); // SCK low
	*/
	// initialize SPI interface
	// master mode and Fosc/2 clock:
	::Register::SPCR = (1 << SPE) | (1 << MSTR);
	SPSR |= (1<<SPI2X);
	// perform system reset
	WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	_delay_ms(50);
	// check CLKRDY bit to see if reset is complete
	// The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
	//while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// set receive buffer start address
	NextPacketPtr = RXSTART_INIT;
	// Rx start
	Write(ERXSTL, RXSTART_INIT&0xFF);
	Write(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	Write(ERXRDPTL, RXSTART_INIT&0xFF);
	Write(ERXRDPTH, RXSTART_INIT>>8);
	// RX end
	Write(ERXNDL, RXSTOP_INIT&0xFF);
	Write(ERXNDH, RXSTOP_INIT>>8);
	// TX start
	Write(ETXSTL, TXSTART_INIT&0xFF);
	Write(ETXSTH, TXSTART_INIT>>8);
	// TX end
	Write(ETXNDL, TXSTOP_INIT&0xFF);
	Write(ETXNDH, TXSTOP_INIT>>8);
	// do bank 1 stuff, packet filter:
	// For broadcast packets we allow only ARP packtets
	// All other packets should be unicast only for our mac (MAADR)
	//
	// The pattern to match on is therefore
	// Type     ETH.DST
	// ARP      BROADCAST
	// 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
	// in binary these poitions are:11 0000 0011 1111
	// This is hex 303F->EPMM0=0x3f,EPMM1=0x30
	Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
	Write(EPMM0, 0x3f);
	Write(EPMM1, 0x30);
	Write(EPMCSL, 0xf9);
	Write(EPMCSH, 0xf7);
    
	// do bank 2 stuff
	// enable MAC receive
	Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	// bring MAC out of reset
	Write(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	WriteOp(ENC28J60_BIT_FIELD_SET, MACON3,
			MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
	// set inter-frame gap (non-back-to-back)
	Write(MAIPGL, 0x12);
	Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	Write(MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
	// Do not send packets longer than MAX_FRAMELEN:
	Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
	Write(MAMXFLH, MAX_FRAMELEN>>8);
	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	Write(MAADR5, macaddr[0]);
	Write(MAADR4, macaddr[1]);
	Write(MAADR3, macaddr[2]);
	Write(MAADR2, macaddr[3]);
	Write(MAADR1, macaddr[4]);
	Write(MAADR0, macaddr[5]);
	// no loopback of transmitted frames
	PhyWrite(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	SetBank(ECON1);
	// enable interrutps
	WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	// enable packet reception
	WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	}
    // read the revision of the chip:
    static uint8_t getrev(void)
	{
	return Read(EREVID);
	}
    static void PacketSend(uint16_t len, uint8_t* packet)
	{
	// Set the write pointer to start of transmit buffer area
	Write(EWRPTL, TXSTART_INIT&0xFF);
	Write(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	Write(ETXNDL, (TXSTART_INIT+len)&0xFF);
	Write(ETXNDH, (TXSTART_INIT+len)>>8);
	// write per-packet control byte (0x00 means use macon3 settings)
	WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	// copy the packet into the transmit buffer
	WriteBuffer(len, packet);
	// send the contents of the transmit buffer onto the network
	WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
	// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
	if(Read(EIR) & EIR_TXERIF)
	    WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}
    // Gets a packet from the network receive buffer, if one is available.
    // The packet will by headed by an ethernet header.
    //      maxlen  The maximum acceptable length of a retrieved packet.
    //      packet  Pointer where packet data should be stored.
    // Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
    static uint16_t PacketReceive(uint16_t maxlen, uint8_t* packet)
	{
	uint16_t rxstat;
	uint16_t len;
	// check if a packet has been received and buffered
	//if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
	// The above does not work. See Rev. B4 Silicon Errata point 6.
	if (Read(EPKTCNT) == 0)
	    return 0;
	
	// Set the read pointer to the start of the received packet
	Write(ERDPTL, (NextPacketPtr));
	Write(ERDPTH, (NextPacketPtr)>>8);
	// read the next packet pointer
	NextPacketPtr  = ReadOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	// read the packet length (see datasheet page 43)
	len  = ReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	len -= 4; //remove the CRC count
	// read the receive status (see datasheet page 43)
	rxstat  = ReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	// limit retrieve length
	if (len > maxlen-1)
	    len=maxlen-1;
	// check CRC and symbol errors (see datasheet page 44, table 7-3):
	// The ERXFCON.CRCEN is set by default. Normally we should not
	// need to check this.
	if ((rxstat & 0x80) == 0)
	    {
	    // invalid
	    len=0;
	    }
	else
	    {
	    // copy the packet from the receive buffer
	    ReadBuffer(len, packet);
	    }
	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	Write(ERXRDPTL, (NextPacketPtr));
	Write(ERXRDPTH, (NextPacketPtr)>>8);
	// decrement the packet counter indicate we are done with this packet
	WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return len;
	}
    static void phlcon(uint16_t val) { PhyWrite(PHLCON, val); }
    static void setup(const byte *mac)
	{
	Init(mac);
	clkout(2); // change clkout from 6.25MHz to 12.5MHz
	_delay_ms(10);
        
	/* Magjack leds configuration, see enc28j60 datasheet, page 11 */
	// LEDA=greed LEDB=yellow

	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	phlcon(0x880);
	_delay_ms(500);

	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	phlcon(0x990);
	_delay_ms(500);

	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	phlcon(0x880);
	_delay_ms(500);

	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	phlcon(0x990);
	_delay_ms(500);

	// 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	// enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	phlcon(0x476);
	_delay_ms(100);
	}

private:
    static const byte ADDR_MASK = 0x1f;
    static const byte BANK_MASK = 0x60;
    static const byte RXSTART_INIT = 0x0;
    static const uint16_t RXSTOP_INIT = 0x1FFF-0x0600-1;
    static const uint16_t TXSTART_INIT = 0x1FFF-0x0600;
    static const uint16_t TXSTOP_INIT = 0x1FFF;
    static const uint16_t MAX_FRAMELEN = 1500;  // (note: maximum ethernet frame
    enum Opcode
	{
	ENC28J60_READ_CTRL_REG = 0x00,
	ENC28J60_READ_BUF_MEM = 0x3A,
	ENC28J60_WRITE_CTRL_REG = 0x40,
	ENC28J60_WRITE_BUF_MEM = 0x7A,
	ENC28J60_BIT_FIELD_SET = 0x80,
	ENC28J60_BIT_FIELD_CLR = 0xA0,
	ENC28J60_SOFT_RESET = 0xFF,
	};
    enum Register
	{
	PHCON2 = 0x10,
	PHLCON = 0x14,
	EIE = 0x1B,
	EIR = 0x1C,
	ECON2 = 0x1E,
	ECON1 = 0x1F,
	ERDPTL =   (0x00|0x00),
	ERDPTH =   (0x01|0x00),
	EWRPTL =   (0x02|0x00),
	EWRPTH =   (0x03|0x00),
	ETXSTL =   (0x04|0x00),
	ETXSTH =   (0x05|0x00),
	ETXNDL =   (0x06|0x00),
	ETXNDH =   (0x07|0x00),
	ERXSTL =   (0x08|0x00),
	ERXSTH =   (0x09|0x00),
	ERXNDL =   (0x0A|0x00),
	ERXNDH =   (0x0B|0x00),
	ERXRDPTL = (0x0C|0x00),
	ERXRDPTH = (0x0D|0x00),
	EPMM0 =    (0x08|0x20),
	EPMM1 =    (0x09|0x20),
	EPMCSL =   (0x10|0x20),
	EPMCSH =   (0x11|0x20),
	ERXFCON =  (0x18|0x20),
	EPKTCNT =  (0x19|0x20),
	ECOCON =   (0x15|0x60),
	MACON1 =   (0x00|0x40|0x80),
	MACON2 =   (0x01|0x40|0x80),
	MACON3 =   (0x02|0x40|0x80),
	MABBIPG =  (0x04|0x40|0x80),
	MAIPGL =   (0x06|0x40|0x80),
	MAIPGH =   (0x07|0x40|0x80),
	MAMXFLL =  (0x0A|0x40|0x80),
	MAMXFLH =  (0x0B|0x40|0x80),
	MIREGADR = (0x14|0x40|0x80),
	MIWRL =    (0x16|0x40|0x80),
	MIWRH =    (0x17|0x40|0x80),
	MAADR1 =   (0x00|0x60|0x80),
	MAADR0 =   (0x01|0x60|0x80),
	MAADR3 =   (0x02|0x60|0x80),
	MAADR2 =   (0x03|0x60|0x80),
	MAADR5 =   (0x04|0x60|0x80),
	MAADR4 =   (0x05|0x60|0x80),
	MISTAT =   (0x0A|0x60|0x80),
	EREVID =   (0x12|0x60),
	};
    enum ECON1Bit
	{
	ECON1_TXRTS = 0x08,
	ECON1_RXEN = 0x04,
	ECON1_BSEL1 = 0x02,
	ECON1_BSEL0 = 0x01,
	};
    enum ECON2Bit
	{
	ECON2_PKTDEC = 0x40,
	};
    enum MISTATBit
	{
	MISTAT_BUSY = 0x01,
	};
    enum ERXFCONBit
	{
	ERXFCON_UCEN = 0x80,
	ERXFCON_CRCEN = 0x20,
	ERXFCON_PMEN = 0x10,
	};
    enum MACON1Bit
	{
	MACON1_TXPAUS = 0x08,
	MACON1_RXPAUS = 0x04,
	MACON1_MARXEN = 0x01,
	};
    enum MACON3Bit
	{
	MACON3_PADCFG0 = 0x20,
	MACON3_TXCRCEN = 0x10,
	MACON3_FRMLNEN = 0x02,
	};
    enum PHCON2Bit
	{
	PHCON2_HDLDIS = 0x0100,
	};
    enum EIEBit
	{
	EIE_INTIE = 0x80,
	EIE_PKTIE = 0x40,
	};
    enum EIRBit
	{
	EIR_TXERIF = 0x02,
	};

    static byte Enc28j60Bank;
    static uint16_t NextPacketPtr;
    };

template<class Pin> byte ENC28J60<Pin>::Enc28j60Bank;
template<class Pin> uint16_t ENC28J60<Pin>::NextPacketPtr;
