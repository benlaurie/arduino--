#include "arduino--.h"
#include "ip.h"
#include "rf12star.h"
#include "serial.h"
#include "tcp_server.h"

class SerialObserver
    {
public:
    static void cantSend() { Serial.write('.'); }
    static void gotPacket(byte id, byte type, byte length, const byte *data)
	{
	Serial.write("Got packet, id: ");
	Serial.writeDecimal(id);
	Serial.write(" type: ");
	Serial.writeDecimal(type);
	Serial.write(" data: ");
	Serial.writeHex(data, length);
	Serial.write("\r\n");
	}
    static void idSent(byte id, byte length, const byte *mac)
	{
	Serial.write("ID sent, id: ");
	Serial.writeDecimal(id);
	Serial.write(" mac: ");
	Serial.writeHex(mac, length);
	Serial.write("\r\n");
	}
    static void sentPacket(byte id, byte type, byte length, const byte *data)
	{
	Serial.write("Sent packet, id: ");
	Serial.writeDecimal(id);
	Serial.write(" type: ");
	Serial.writeDecimal(type);
	Serial.write(" data: ");
	Serial.writeHex(data, length);
	Serial.write("\r\n");
	}
    };

typedef StarMaster<RF12Star, SerialObserver> Master;

typedef ENC28J60<Pin::B0> Ethernet;

typedef IP<Ethernet> MyIP;

static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24}; 
static uint8_t myip[4] = {192,168,1,111};

void setup()
    {
    /*initialize enc28j60*/
    Ethernet::setup(mymac);

    //init the ethernet/ip layer:
    MyIP::init_ip_arp_udp_tcp(mymac, myip);
    }

class MyTCPServer : public TCPServer<MyIP, 80>
    {
    void packetReceived();
    };

void MyTCPServer::packetReceived()
    {
    clearBuffer();
    char *buf = getData();
    if (strncmp("GET / ", buf, 6) != 0)
	// head, post and other methods for possible status codes see:
	// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
	add_p(PSTR("HTTP/1.0 501 Not OK\r\nContent-Type: text/html\r\n\r\n"));
    else
	add_p(PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nHi mum"));
    }

// FIXME: why do I need this?
extern "C" void __cxa_pure_virtual()
    {
    Serial.write("Ooops!");
    for( ; ; )
	;
    }

int main()
    {
    MyTCPServer tcp;

    Arduino::init();
    Serial.begin(57600);
    Master::init();
    setup();

    Serial.write("Start\r\n");

    for ( ; ; )
	{
	Master::poll();
	tcp.poll();
	}
    }
