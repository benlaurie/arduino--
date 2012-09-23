#include <string.h>
#include "ip.h"

// EtherShield
//typedef ENC28J60<Pin::B2> Ethernet;
// Nanode
typedef ENC28J60<Pin::B0> Ethernet;

typedef IP<Ethernet, 80> IP80;

static uint16_t mywwwport = 80; // listen port for tcp/www (max range 1-254)

static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24}; 
static uint8_t myip[4] = {192,168,1,111};

void setup()
    {
    /*initialize enc28j60*/
    Ethernet::setup(mymac);

    //init the ethernet/ip layer:
    IP80::init_ip_arp_udp_tcp(mymac,myip);
    }

static char hexdigit(byte b)
    {
    if (b < 10)
	return '0' + b;
    return 'a' + b - 10;
    }

class TCPBuffer
    {
public:
    TCPBuffer()
      : len_(0)
	{}

    void add_p(const char *pmem)
	{ len_ = IP80::fill_tcp_data_p(buf, len_, pmem); }
    void add(const char *str)
	{ len_ = IP80::fill_tcp_data(buf, len_, str); }
    void add_hex(byte b)
	{
	char buf[3];

	buf[0] = hexdigit(b >> 4);
	buf[1] = hexdigit(b & 0xf);
	buf[2] = '\0';
	add(buf);
	}
    void add_bit(int bit)
	{
	char buf[2];

	buf[0] = '0' + bit;
	buf[1] = '\0';
	add(buf);
	}
    uint16_t length() const
	{ return len_; }
    void poll();

private:
    uint16_t len_;
    static const uint16_t BUFFER_SIZE = 500;
    uint8_t buf[BUFFER_SIZE + 1];
    };

uint16_t print_webpage(TCPBuffer *tcp)
    {
    tcp->add_p(PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nHi mum"));
    return tcp->length();
    }

void TCPBuffer::poll()
    {
    uint16_t plen, dat_p;

    plen = Ethernet::PacketReceive(BUFFER_SIZE, buf);

    /* plen will be unequal to zero if there is a valid packet
       (without crc error) */
    if (plen != 0)
	{
	// arp is broadcast if unknown but a host may also verify the
	// mac address by sending it to a unicast address.
	if(IP80::eth_type_is_arp_and_my_ip(buf,plen))
	    {
	    IP80::make_arp_answer_from_request(buf);
	    return;
	    }

	// check if ip packets are for us:
	if(IP80::eth_type_is_ip_and_my_ip(buf,plen) == 0)
	    return;
    
	if(buf[IP_PROTO_P] == IP_PROTO_ICMP_V
	   && buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V)
	    {
	    IP80::make_echo_reply_from_request(buf,plen);
	    return;
	    }
    
	// tcp port www start, compare only the lower byte
	if (buf[IP_PROTO_P] == IP_PROTO_TCP_V
	    && buf[TCP_DST_PORT_H_P] == 0
	    && buf[TCP_DST_PORT_L_P] == mywwwport)
	    {
	    if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
		{
		IP80::make_tcp_synack_from_syn(buf); // make_tcp_synack_from_syn does already send the syn,ack
		return;
		}
	    if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
		{
		IP80::init_len_info(buf); // init some data structures
		dat_p=IP80::get_tcp_data_pointer();
		if (dat_p == 0)
		    { // we can possibly have no data, just ack:
		    if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
			IP80::make_tcp_ack_from_any(buf);
		    return;
		    }
		if (strncmp("GET ",(char *)&(buf[dat_p]),4) != 0)
		    {
		    // head, post and other methods for possible status codes see:
		    // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
		    plen=IP80::fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 501 OK\r\nContent-Type: text/html\r\n\r\n"));
		    goto SENDTCP;
		    }
		if (strncmp("/ ",(char *)&(buf[dat_p+4]),2) == 0)
		    {
		    plen=print_webpage(this);
		    goto SENDTCP;
		    }

	    SENDTCP:
		IP80::make_tcp_ack_from_any(buf); // send ack for http get
		IP80::make_tcp_ack_with_data(buf,plen); // send data       
		}
	    }
	}
    }


int main()
    {
    TCPBuffer tcp;
        
    setup();

    for( ; ; )
	tcp.poll();
    
    return 0;
    }
