#include "arduino--.h"
#include "ip.h"
#include "serial.h"
#include "tcp_server.h"
#include <string.h>

typedef Pin::D4 Zero;  // convenient so we have GND on the next pin
typedef Pin::D5 LEDS;
static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x25}; 
static uint8_t myip[4] = {192,168,1,112};

#define NOP 	__asm__("\tnop\n")

class SlowLEDController
    {
public:
    static void One() __attribute__((always_inline))
	{
	// High for 1.2 us, low for 1.3 us
	NOP;
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastOne(byte **pb) __attribute__((always_inline))
	{
	// High for 1.2 us, low for 1.3 us
	NOP;
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	++*pb;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void Zero() __attribute__((always_inline))
	{
	// High for .5 us, low for 2 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastZero(byte **pb) __attribute__((always_inline))
	{
	// High for .5 us, low for 2 us
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	++*pb;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void Reset()
	{
	LEDS::clear();
	_delay_us(50);
	}
    };

// fast
class LEDController
    {
public:
    static void One() __attribute__((always_inline))
	{
	// High for .6 us, low for .65 us
	NOP;
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastOne(byte **pb) __attribute__((always_inline))
	{
	// High for .6 us, low for .65 us
	NOP;
	LEDS::set();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	LEDS::clear();
	++*pb;
	}

    static void Zero() __attribute__((always_inline))
	{
	// High for .25 us, low for 1 us
	LEDS::set();
	NOP;
	NOP;
	LEDS::clear();
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	}

    static void LastZero(byte **pb) __attribute__((always_inline))
	{
	// High for .25 us, low for 1 us
	LEDS::set();
	NOP;
	NOP;
	LEDS::clear();
	++*pb;
	NOP;
	NOP;
	}

    static void Reset()
	{
	LEDS::clear();
	_delay_us(50);
	}
    };

typedef ENC28J60<Pin::B0> Ethernet;
typedef IP<Ethernet> MyIP;

static bool waiting = true;

static byte base[] = {
#include "test.grb"
};

static byte buf[sizeof base];

class MyTCPServer : public TCPServer<MyIP, 222>
    {
public:
    MyTCPServer() : offset_(0)
	{}

private:
    void packetReceived()
	{
	byte acks = 0;
	clearBuffer();

	waiting = false;

	size_t len = getDataLength();
	size_t pos = 0;
	//Serial.writeDecimal(getDataLength());
	//Serial.write("/");
	//Serial.writeHex((byte *)getData(), getDataLength());
	while(len > 0)
	    {
	    size_t n;
	    if (offset_ + len >= sizeof buf)
		{
		n = sizeof buf - offset_;
		++acks;
		}
	    else
		n = len;

	    memcpy(buf + offset_, getData() + pos, n);

	    offset_ += n;
	    if (offset_ == sizeof buf)
		offset_ = 0;
	    pos += n;
	    len -= n;
	    }
	while(acks-- > 0)
	    add((byte *)".", 1);
	}

    size_t offset_;
    };

int main(void)
    {
    Nanode::init();
    Serial.begin(57600);

    /*initialize enc28j60*/
    Ethernet::setup(mymac);

    //init the ethernet/ip layer:
    MyIP::init_ip_arp_udp_tcp(mymac, myip);

    MyTCPServer tcp;

    Zero::modeOutput();
    Zero::clear();
    LEDS::modeOutput();
    //    buf[721] = 0xff;
    for (uint16_t offset = 0 ; ; offset += 3)
	{
	tcp.poll();
#if 0
	// For some weird reason this kills Ethernet!
	if (waiting)
	    {
	    for (uint16_t n = 0; n < sizeof buf; ++n)
		buf[n] = base[(n + offset) % sizeof buf];
	    if (offset >= sizeof buf)
		offset = 0;
	    }
#endif
	//Serial.write("/");
	//Serial.writeHex(buf, 255);
	byte *pb = buf;
	byte *pe = buf + sizeof buf;
	LEDController::Reset();
	for (; pb < pe; )
	    {
	    byte b = *pb;
#define F(x) \
	    if (b & x) \
		LEDController::One(); \
	    else \
		LEDController::Zero()
	    F(0x80);
	    F(0x40);
	    F(0x20);
	    F(0x10);
	    F(0x08);
	    F(0x04);
	    F(0x02);
	    //F(0x01); ++pb;
	    if (b & 0x01)
		LEDController::LastOne(&pb);
	    else
		LEDController::LastZero(&pb);
	    }
	}
    }

// FIXME: why do I need this?
extern "C" void __cxa_pure_virtual()
    {
    for( ; ; )
	;
    }
