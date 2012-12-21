#include "arduino--.h"
#include "ip.h"
#include "serial.h"
#include "tcp_server.h"
#include "ws2811.h"
#include <string.h>

typedef Pin::D4 Zero;  // convenient so we have GND on the next pin
typedef Pin::D5 LEDS;
DEFINE_WS2811_FN(WS2811RGB, PORTD, 5)

static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x25}; 
static uint8_t myip[4] = {192,168,1,112};

typedef ENC28J60<Pin::B0> Ethernet;
typedef IP<Ethernet> MyIP;

static bool waiting = true;

static byte base[] = {
#include "test.rgb"
};

static byte buf[sizeof base + 3];

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
	    //if (offset_ + len >= sizeof buf)
	    if (offset_ + len >= 720)
		{
		//n = sizeof buf - offset_;
		n = 720 - offset_;
		++acks;
		}
	    else
		n = len;

	    memcpy(buf + offset_, getData() + pos, n);

	    offset_ += n;
	    //	    if (offset_ == sizeof buf)
	    if (offset_ == 720)
		offset_ = 0;
	    pos += n;
	    len -= n;
	    }

	while (acks-- > 0)
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
    buf[721] = 0xff;
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
	Serial.write("/");
	Serial.writeHex(buf, 3);
	_delay_us(50);
	WS2811RGB((RGB_t *)buf, sizeof buf/3);
	}
    }

// FIXME: why do I need this?
extern "C" void __cxa_pure_virtual()
    {
    for( ; ; )
	;
    }
