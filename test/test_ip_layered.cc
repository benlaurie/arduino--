#include <string.h>
#include "ip.h"
#include "tcp_server.h"

// EtherShield
//typedef ENC28J60<Pin::B2> Ethernet;
// Nanode
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
extern "C" void __cxa_pure_virtual() { while (1); }

int main()
    {
    MyTCPServer tcp;
        
    setup();

    for( ; ; )
	tcp.poll();
    
    return 0;
    }
