// -*- mode: c++; indent-tabs-mode: nil; -*-

// FIXME: this should not be here
static char hexdigit(byte b)
    {
    if (b < 10)
	return '0' + b;
    return 'a' + b - 10;
    }

template <class MyIP, byte port> class TCPServer
    {
public:
    TCPServer()
      : len_(0)
	{}

    void add_p(const char *pmem)
	{ len_ = MyIP::fill_tcp_data_p(buf_, len_, pmem); }
    void add(const char *str)
	{ len_ = MyIP::fill_tcp_data(buf_, len_, str); }
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
    void clearBuffer()
	{ len_ = 0; }
    char *getData() const
	{ return (char *)&(buf_[MyIP::get_tcp_data_pointer()]); }
    void poll();

private:
    virtual void packetReceived() = 0;

    uint16_t len_;
    static const uint16_t BUFFER_SIZE = 500;
    uint8_t buf_[BUFFER_SIZE + 1];
    };

template <class MyIP, byte port> void TCPServer<MyIP, port>::poll()
    {
    uint16_t plen, dat_p;

    plen = MyIP::PacketReceive(BUFFER_SIZE, buf_);

    /* plen will be unequal to zero if there is a valid packet
       (without crc error) */
    if (plen != 0)
	{
	// arp is broadcast if unknown but a host may also verify the
	// mac address by sending it to a unicast address.
	if(MyIP::eth_type_is_arp_and_my_ip(buf_, plen))
	    {
	    MyIP::make_arp_answer_from_request(buf_);
	    return;
	    }

	// check if ip packets are for us:
	if(MyIP::eth_type_is_ip_and_my_ip(buf_, plen) == 0)
	    return;
    
	if(buf_[IP_PROTO_P] == IP_PROTO_ICMP_V
	   && buf_[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V)
	    {
	    MyIP::make_echo_reply_from_request(buf_, plen);
	    return;
	    }
    
	// tcp port www start, compare only the lower byte
	if (buf_[IP_PROTO_P] == IP_PROTO_TCP_V
	    && buf_[TCP_DST_PORT_H_P] == 0
	    && buf_[TCP_DST_PORT_L_P] == port)
	    {
	    if (buf_[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
		{
		MyIP::make_tcp_synack_from_syn(buf_, port); // make_tcp_synack_from_syn does already send the syn,ack
		return;
		}
	    if (buf_[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
		{
		MyIP::init_len_info(buf_); // init some data structures
		dat_p = MyIP::get_tcp_data_pointer();
		if (dat_p == 0)
		    { // we can possibly have no data, just ack:
		    if (buf_[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
			MyIP::make_tcp_ack_from_any(buf_, port);
		    return;
		    }

		packetReceived();

		MyIP::make_tcp_ack_from_any(buf_, port); // send ack for http get
		MyIP::make_tcp_ack_with_data(buf_, len_); // send data
		}
	    }
	}
    }
