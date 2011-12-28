// -*- mode: c++; indent-tabs-mode: nil; -*-
// RFM12B driver definitions
// 2009-02-09 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// 2011-12-28 Ben Laurie <ben@links.org>

#ifndef RF12_h
#define RF12_h

#include <stdint.h>
#include <string.h>

#include <util/crc16.h>

#include "arduino++.h"
#include "spi.h"

// version 1 did not include the group code in the crc
// version 2 does include the group code in the crc
#define RF12_VERSION    2

#define RF12_HDR_CTL    0x80
#define RF12_HDR_DST    0x40
#define RF12_HDR_ACK    0x20
#define RF12_HDR_MASK   0x1F

#define RF12_MAXDATA    66

// EEPROM address range used by the rf12_config() code
#define RF12_EEPROM_ADDR ((uint8_t*) 0x20)
#define RF12_EEPROM_SIZE 32
#define RF12_EEPROM_EKEY (RF12_EEPROM_ADDR + RF12_EEPROM_SIZE)
#define RF12_EEPROM_ELEN 16

// shorthand to simplify sending out the proper ACK when requested
#define RF12_WANTS_ACK ((rf12_hdr & RF12_HDR_ACK) && !(rf12_hdr & RF12_HDR_CTL))
#define RF12_ACK_REPLY (rf12_hdr & RF12_HDR_DST ? RF12_HDR_CTL : \
            RF12_HDR_CTL | RF12_HDR_DST | (rf12_hdr & RF12_HDR_MASK))
            
// options for RF12_sleep()
#define RF12_SLEEP 0
#define RF12_WAKEUP -1

#define OPTIMIZE_SPI 1   // uncomment this to write to the RFM12B @ 8 Mhz

// maximum transmit / receive buffer: 3 header + data + 2 crc bytes
#define RF_MAX   (RF12_MAXDATA + 5)

// pins used for the RFM12B interface - yes, there *is* logic in this madness:
//
//  - leave RFM_IRQ set to the pin which corresponds with INT0, because the
//    current driver code will use attachInterrupt() to hook into that
//  - use SS_DDR, SS_PORT, and SS_BIT to define the pin you will be using as
//    select pin for the RFM12B (you're free to set them to anything you like)
//  - please leave SPI_SS, SPI_MOSI, SPI_MISO, and SPI_SCK as is, i.e. pointing
//    to the hardware-supported SPI pins on the ATmega, *including* SPI_SS !

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)

#define RFM_IRQ     2
#define SS_DDR      DDRB
#define SS_PORT     PORTB
#define SS_BIT      0

#define SPI_SS      53    // PB0, pin 19
#define SPI_MOSI    51    // PB2, pin 21
#define SPI_MISO    50    // PB3, pin 22
#define SPI_SCK     52    // PB1, pin 20

#elif defined(__AVR_ATmega644P__)

#define RFM_IRQ     10
#define SS_DDR      DDRB
#define SS_PORT     PORTB
#define SS_BIT      4

#define SPI_SS      4
#define SPI_MOSI    5
#define SPI_MISO    6
#define SPI_SCK     7

#elif defined(__AVR_ATtiny84__)

#define RFM_IRQ     2
#define SS_DDR      DDRB
#define SS_PORT     PORTB
#define SS_BIT      1

#define SPI_SS      1     // PB1, pin 3
#define SPI_MISO    4     // PA6, pin 7
#define SPI_MOSI    5     // PA5, pin 8
#define SPI_SCK     6     // PA4, pin 9

#else

// ATmega168, ATmega328, etc.
//#define RFM_IRQ     2
#define SS_DDR      DDRB
#define SS_PORT     PORTB
#define SS_BIT      2     // for PORTB: 2 = d.10, 1 = d.9, 0 = d.8

//#define SPI_SS      10    // PB2, pin 16
#define SPI_MOSI    11    // PB3, pin 17
#define SPI_MISO    12    // PB4, pin 18
#define SPI_SCK     13    // PB5, pin 19

#endif 

// RF12 command codes
#define RF_RECEIVER_ON  0x82DD
#define RF_XMITTER_ON   0x823D
#define RF_IDLE_MODE    0x820D
#define RF_SLEEP_MODE   0x8205
#define RF_WAKEUP_MODE  0x8207
#define RF_TXREG_WRITE  0xB800
#define RF_RX_FIFO_READ 0xB000
#define RF_WAKEUP_TIMER 0xE000

// RF12 status bits
#define RF_LBD_BIT      0x0400
#define RF_RSSI_BIT     0x0100

// bits in the node id configuration byte
#define NODE_BAND       0xC0        // frequency band
#define NODE_ACKANY     0x20        // ack on broadcast packets if set
#define NODE_ID         0x1F        // id of this node, as A..Z or 1..31

template <class RFM_IRQ> class _RF12
    {
    static volatile uint16_t _crc;  // running crc value, should be
                                    // zero at end
    enum BufferOffset
        {
        GROUP = 0,
        HEADER = 1,
        LENGTH = 2,
        DATA = 3
        };
    static volatile byte _buf[]; // recv/xmit buf including hdr &
                                    // crc bytes
    static long _seq;               // seq number of encrypted packet (or -1)

    // transceiver states, these determine what to do with each interrupt
    enum TransceiverState
        {
        TXCRC1,
        TXCRC2,
        TXTAIL,
        TXDONE,
        TXIDLE,
        TXRECV,
        TXPRE1,
        TXPRE2,
        TXPRE3,
        TXSYN1,
        TXSYN2,
        };

    static uint8_t _nodeid;              // address of this node
    static uint8_t _group;               // network group
    static volatile byte _rxfill;     // number of data bytes in rf12_buf
    static volatile int8_t _rxstate;     // current transceiver state

#define RETRIES     8               // stop retrying after 8 times
#define RETRY_MS    1000            // resend packet every second until ack'ed

    static uint8_t _ezInterval;          // number of seconds between transmits
    static uint8_t _ezSendBuf[RF12_MAXDATA]; // data to send
    static char _ezSendLen;              // number of bytes to send
    static uint8_t _ezPending;           // remaining number of retries
    static long _ezNextSend[2];          // when was last retry [0] or
                                         // data [1] sent

    static uint32_t _seqNum;             // encrypted send sequence number
    static uint32_t _cryptKey[4];        // encryption key to use
    static void (*_crypter)(uint8_t);    // does en-/decryption (null
                                         // if disabled)

public:
    // only needed if you want to init the SPI bus before
    // rf12_initialize does it
    static void spiInit(void)
        {
        // maybe use clk/2 (2x 1/4th) for sending (and clk/8 for recv,
        // see rf12_xferSlow)
        SPISS::init(0, F_CPU > 10000000);

        RFM_IRQ::modeInput();
        // pullup (apparently)
        RFM_IRQ::set();
        }

    enum Band
        {
        MHZ433 = 1,
        MHZ868 = 2,
        MHZ915 = 3,
        };

    // call this once with the node ID, frequency band, and optional group
    static void init(uint8_t id, uint8_t band, uint8_t group = 0xD4)
        {
        _nodeid = id;
        _group = group;
    
        spiInit();

        xfer(0x0000); // intitial SPI transfer added to avoid power-up problem
        xfer(RF_SLEEP_MODE); // DC (disable clk pin), enable lbd
    
        // wait until RFM12B is out of power-up reset, this takes
        // several *seconds*
        xfer(RF_TXREG_WRITE); // in case we're still in OOK mode
        while (RFM_IRQ::read() == 0)
            xfer(0x0000);
        
        xfer(0x80C7 | (band << 4));      // EL (ena TX), EF (ena RX
                                         // FIFO), 12.0pF
        xfer(0xA640); // 868MHz 
        xfer(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
        xfer(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm 
        xfer(0xC2AC); // AL,!ml,DIG,DQD4 
        if (group != 0)
            {
            xfer(0xCA83); // FIFO8,2-SYNC,!ff,DR 
            xfer(0xCE00 | group); // SYNC=2DXX； 
            }
        else
            {
            xfer(0xCA8B); // FIFO8,1-SYNC,!ff,DR 
            xfer(0xCE2D); // SYNC=2D； 
            }
        xfer(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN 
        xfer(0x9850); // !mp,90kHz,MAX OUT 
        xfer(0xCC77); // OB1，OB0, LPX,！ddy，DDIT，BW0 
        xfer(0xE000); // NOT USE 
        xfer(0xC800); // NOT USE 
        xfer(0xC049); // 1.66MHz,3.1V 

        _rxstate = TXIDLE;
        if ((_nodeid & NODE_ID) != 0)
            Interrupt0::enable(Interrupt0::LOW);
        else
            Interrupt0::disable();
        }

    // call this frequently, returns true if a packet has been received
    static bool recvDone(void)
        {
        if (_rxstate == TXRECV && (_rxfill >= _buf[LENGTH] + 5
                                   || _rxfill >= RF_MAX))
            {
            _rxstate = TXIDLE;
            if (_buf[LENGTH] > RF12_MAXDATA)
                _crc = 1; // force bad crc if packet length is invalid
            if (!(_buf[HEADER] & RF12_HDR_DST) || (_nodeid & NODE_ID) == 31 ||
                (_buf[HEADER] & RF12_HDR_MASK) == (_nodeid & NODE_ID))
                {
                if (_crc == 0 && _crypter != 0)
                    _crypter(0);
                else
                    _seq = -1;
                return true; // it's a broadcast packet or it's addressed
                          // to this node
                }
            }
        if (_rxstate == TXIDLE)
            recvStart();
        return false;
        }

    static bool goodCRC() { return _crc == 0; }
    static byte header() { return _buf[HEADER]; }
    static byte length() { return _buf[LENGTH]; }
    static const volatile byte *data() { return &_buf[DATA]; }

    // call this to check whether a new transmission can be started
    // returns true when a new transmission may be started with
    // rf12_sendStart()
    static bool canSend(void)
        {
        // no need to test with interrupts disabled: state TXRECV is
        // only reached outside of ISR and we don't care if rxfill
        // jumps from 0 to 1 here
        if (_rxstate == TXRECV && _rxfill == 0 &&
            (xferByte(0x00) & (RF_RSSI_BIT >> 8)) == 0)
            {
            xfer(RF_IDLE_MODE); // stop receiver
            //XXX just in case, don't know whether these RF12 reads are needed!
            // rf12_xfer(0x0000); // status register
            // rf12_xfer(RF_RX_FIFO_READ); // fifo read
            _rxstate = TXIDLE;
            _buf[GROUP] = _group;
            return true;
            }
        return false;
        }

    // returns true if the buffer currently contains a packet that
    // needs ACKing.
    static bool wantsAck()
        {
        return (_buf[HEADER] & RF12_HDR_ACK) && !(_buf[HEADER] & RF12_HDR_CTL);
        }

    // Send an ack reply to the packet in the buffer (wantsAck() must be true)
    static void sendAckReply()
        {
        byte hdr;

        if (_buf[HEADER] & RF12_HDR_DST)
            hdr = RF12_HDR_CTL;
        else
            hdr = RF12_HDR_CTL | RF12_HDR_DST | (_buf[HEADER] & RF12_HDR_MASK);
        sendStart(hdr, 0, 0);
        }
    static bool isAckReply() { return (header() & RF12_HDR_CTL) != 0; }

    // call this only when rf12_recvDone() or rf12_canSend() return true
    static void sendStart(uint8_t hdr)
        {
        _buf[HEADER] = hdr & RF12_HDR_DST ? hdr :
            (hdr & ~RF12_HDR_MASK) + (_nodeid & NODE_ID);
        if (_crypter != 0)
            _crypter(1);
    
        _crc = ~0;
#if RF12_VERSION >= 2
        _crc = _crc16_update(_crc, _buf[GROUP]);
#endif
        _rxstate = TXPRE1;
        xfer(RF_XMITTER_ON); // bytes will be fed via interrupts
        }

    static void sendStart(uint8_t hdr, const void* ptr, uint8_t len)
        {
        _buf[LENGTH] = len;
        memcpy((void *)&_buf[DATA], ptr, len);
        sendStart(hdr);
        }

// wait for send to finish, sleep mode: 0=none, 1=idle, 2=standby, 3=powerdown
void rf12_sendWait(uint8_t mode);

// this simulates OOK by turning the transmitter on and off via SPI commands
// use this only when the radio was initialized with a fake zero node ID
void rf12_onOff(uint8_t value);

// power off the RF12, ms > 0 sets watchdog to wake up again after N * 32 ms
// note: once off, calling this with -1 can be used to bring the RF12 back up
void rf12_sleep(char n);

// returns true of the supply voltage is below 3.1V
char rf12_lowbat(void);

// set up the easy tranmission mode, arg is number of seconds between packets
void rf12_easyInit(uint8_t secs);

// call this often to keep the easy transmission mode going
char rf12_easyPoll(void);

// send new data using the easy transmission mode, buffer gets copied to driver
char rf12_easySend(const void* data, uint8_t size);

// enable encryption (null arg disables it again)
void rf12_encrypt(const uint8_t*);

// low-level control of the RFM12B via direct register access
// http://tools.jeelabs.org/rfm12b is useful for calculating these
uint16_t rf12_control(uint16_t cmd);

// See http://blog.strobotics.com.au/2009/07/27/rfm12-tutorial-part-3a/
// Transmissions are packetized, don't assume you can sustain these speeds! 
//
// Note - data rates are approximate. For higher data rates you may need to
// alter receiver radio bandwidth and transmitter modulator bandwidth.
// Note that bit 7 is a prescaler - don't just interpolate rates between
// RF12_DATA_RATE_3 and RF12_DATA_RATE_2.
enum rf12DataRates {
    RF12_DATA_RATE_CMD = 0xC600,
    RF12_DATA_RATE_9 = RF12_DATA_RATE_CMD | 0x02,  // Approx 115200 bps
    RF12_DATA_RATE_8 = RF12_DATA_RATE_CMD | 0x05,  // Approx  57600 bps
    RF12_DATA_RATE_7 = RF12_DATA_RATE_CMD | 0x06,  // Approx  49200 bps
    RF12_DATA_RATE_6 = RF12_DATA_RATE_CMD | 0x08,  // Approx  38400 bps
    RF12_DATA_RATE_5 = RF12_DATA_RATE_CMD | 0x11,  // Approx  19200 bps
    RF12_DATA_RATE_4 = RF12_DATA_RATE_CMD | 0x23,  // Approx   9600 bps
    RF12_DATA_RATE_3 = RF12_DATA_RATE_CMD | 0x47,  // Approx   4800 bps
    RF12_DATA_RATE_2 = RF12_DATA_RATE_CMD | 0x91,  // Approx   2400 bps
    RF12_DATA_RATE_1 = RF12_DATA_RATE_CMD | 0x9E,  // Approx   1200 bps
    RF12_DATA_RATE_DEFAULT = RF12_DATA_RATE_7,
};

    static void interrupt()
        {
        // a transfer of 2x 16 bits @ 2 MHz over SPI takes 2x 8 us
        // inside this ISR correction: now takes 2 + 8 µs, since
        // sending can be done at 8 MHz
        xfer(0x0000);
    
        if (_rxstate == TXRECV)
            {
            uint8_t in = xferSlow(RF_RX_FIFO_READ);

            if (_rxfill == 0 && _group != 0)
                _buf[_rxfill++] = _group;
            
            _buf[_rxfill++] = in;
            _crc = _crc16_update(_crc, in);

            if (_rxfill >= _buf[LENGTH] + 5 || _rxfill >= RF_MAX)
                xfer(RF_IDLE_MODE);
            }
        else
            {
            uint8_t out;

            if (_rxstate < 0)
                {
                uint8_t pos = 3 + _buf[LENGTH] + _rxstate++;
                out = _buf[pos];
                _crc = _crc16_update(_crc, out);
                }
            else
                switch (_rxstate++)
                    {
                case TXSYN1: out = 0x2D; break;
                case TXSYN2:
                    out = _buf[GROUP];
                    _rxstate = - (2 + _buf[LENGTH]);
                    break;
                case TXCRC1: out = _crc; break;
                case TXCRC2: out = _crc >> 8; break;
                case TXDONE: xfer(RF_IDLE_MODE); // fall through
                default:     out = 0xAA;
                    }
            
            xfer(RF_TXREG_WRITE + out);
            }
        }

private:
    // FIXME: unify with arduino++.h
    static uint8_t xferByte(uint8_t out)
        {
#ifdef SPDR
        SPDR = out;
        // this loop spins 4 usec with a 2 MHz SPI clock
        while (!(SPSR & _BV(SPIF)))
            ;
        return SPDR;
#else
        // ATtiny
        USIDR = out;
        byte v1 = bit(USIWM0) | bit(USITC);
        byte v2 = bit(USIWM0) | bit(USITC) | bit(USICLK);
#if F_CPU <= 5000000
        // only unroll if resulting clock stays under 2.5 MHz
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
        USICR = v1; USICR = v2;
#else
        for (uint8_t i = 0; i < 8; ++i)
            {
            USICR = v1;
            USICR = v2;
            }
#endif
        return USIDR;
#endif
        }

    static uint16_t xferSlow(uint16_t cmd)
        {
        // slow down to under 2.5 MHz
#if F_CPU > 10000000
        //bitSet(SPCR, SPR0);
        //SPCR |= (1 << SPR0);
        Register::SPCR.set(SPR0);
#endif
        //bitClear(SS_PORT, SS_BIT);
        Pin::SPI_SS::clear();
        uint16_t reply = xferByte(cmd >> 8) << 8;
        reply |= xferByte(cmd);
        //bitSet(SS_PORT, SS_BIT);
        Pin::SPI_SS::set();
#if F_CPU > 10000000
        //bitClear(SPCR, SPR0);
        //SPCR &= ~(1 << SPR0);
        Register::SPCR.clear(SPR0);
#endif
        return reply;
        }

    static void xfer(uint16_t cmd)
        {
#if OPTIMIZE_SPI
        // writing can take place at full speed, even 8 MHz works
        //bitClear(SS_PORT, SS_BIT);
        Pin::SPI_SS::clear();
        xferByte(cmd >> 8);
        xferByte(cmd);
        //bitSet(SS_PORT, SS_BIT);
        Pin::SPI_SS::set();
#else
        xferSlow(cmd);
#endif
        }

    static void recvStart ()
        {
        _rxfill = _buf[LENGTH] = 0;
        _crc = ~0;
#if RF12_VERSION >= 2
        if (_group != 0)
            _crc = _crc16_update(~0, _group);
#endif
        _rxstate = TXRECV;
        xfer(RF_RECEIVER_ON);
        }
  
    };

template <class RFM_IRQ> volatile byte _RF12<RFM_IRQ>::_buf[RF_MAX];
template <class RFM_IRQ> volatile byte _RF12<RFM_IRQ>::_rxfill;
template <class RFM_IRQ> volatile uint16_t _RF12<RFM_IRQ>::_crc;
template <class RFM_IRQ> byte _RF12<RFM_IRQ>::_group;
template <class RFM_IRQ> volatile int8_t _RF12<RFM_IRQ>::_rxstate;
template <class RFM_IRQ> byte _RF12<RFM_IRQ>::_nodeid;
template <class RFM_IRQ> void (*_RF12<RFM_IRQ>::_crypter)(byte);
template <class RFM_IRQ> long _RF12<RFM_IRQ>::_seq;

// Setup for Jeenodes and Wi/Nanodes.
typedef _RF12<Pin::D2> RF12B;

SIGNAL(INT0_vect)
    {
    RF12B::interrupt();
    }

#endif
