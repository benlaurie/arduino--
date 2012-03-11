// -*- mode: c++; indent-tabs-mode: nil; -*-
/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 *
 * Author: Guido Socher 
 * Copyright: GPL V2
 * See http://www.gnu.org/licenses/gpl.html
 *
 * IP, Arp, UDP and TCP functions.
 *
 * The TCP implementation uses some size optimisations which are valid
 * only if all data can be sent in one single packet. This is however
 * not a big limitation for a microcontroller as you will anyhow use
 * small web-pages. The TCP stack is therefore a SDP-TCP stack (single data packet TCP).
 *
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/
 /*********************************************
 * Modified: nuelectronics.com -- Ethershield for Arduino
 *********************************************/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "net.h"
#include "enc28j60.h"


template <class Ethernet, byte port> class IP
    {
public:
    // The Ip checksum is calculated over the ip header only starting
   // with the header length field and a total length of 20 bytes
    // unitl ip.dst
    // You must set the IP checksum field to zero before you start
    // the calculation.
    // len for ip is 20.
    //
    // For UDP/TCP we do not make up the required pseudo header. Instead we 
    // use the ip.src and ip.dst fields of the real packet:
    // The udp checksum calculation starts with the ip.src field
    // Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
    // In other words the len here is 8 + length over which you actually
    // want to calculate the checksum.
    // You must set the checksum field to zero before you start
    // the calculation.
    // len for udp is: 8 + 8 + data length
    // len for tcp is: 4+4 + 20 + option len + data length
    //
    // For more information on how this algorithm works see:
    // http://www.netfor2.com/checksum.html
    // http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
    // The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
    static uint16_t checksum(uint8_t *buf, uint16_t len, uint8_t type)
        {
        // type 0=ip 
        //      1=udp
        //      2=tcp
        uint32_t sum = 0;

        //if(type==0){
        //        // do not add anything
        //}
        if (type == 1)
            {
            sum += IP_PROTO_UDP_V; // protocol udp
            // the length here is the length of udp (data+header len)
            // =length given to this function - (IP.scr+IP.dst length)
            sum += len - 8; // = real tcp len
            }
        if (type == 2)
            {
            sum += IP_PROTO_TCP_V; 
            // the length here is the length of tcp (data+header len)
            // =length given to this function - (IP.scr+IP.dst length)
            sum += len-8; // = real tcp len
            }
        // build the sum of 16bit words
        while (len > 1)
            {
            sum += 0xFFFF & (*buf << 8 | *(buf+1));
            buf += 2;
            len -= 2;
            }
        // if there is a byte left then add it (padded with zero)
        if (len)
            sum += (0xFF & *buf) << 8;
        // now calculate the sum over the bytes in the sum
        // until the result is only 16bit long
        while (sum >> 16)
            sum = (sum & 0xFFFF)+(sum >> 16);
        // build 1's complement:
        return (uint16_t) sum ^ 0xFFFF;
        }

    // you must call this function once before you use any of the
    // other functions:
    static void init_ip_arp_udp_tcp(uint8_t *mymac, uint8_t *myip)
        {
        for (byte i = 0; i < 4; ++i)
            ipaddr_[i] = myip[i];
        for (byte i = 0; i < 6; ++i)
            macaddr_[i] = mymac[i];
        }

    static uint8_t eth_type_is_arp_and_my_ip(uint8_t *buf, uint16_t len)
        {
        if (len < 41)
            return 0;
        if (buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V || 
            buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V)
            return 0;
        for(byte i = 0; i < 4; ++i)
            if(buf[ETH_ARP_DST_IP_P+i] != ipaddr_[i])
                return 0;
        return 1;
        }

    static uint8_t eth_type_is_ip_and_my_ip(uint8_t *buf, uint16_t len)
        {
        //eth+ip+udp header is 42
        if (len < 42)
            return 0;
        if (buf[ETH_TYPE_H_P] != ETHTYPE_IP_H_V || 
            buf[ETH_TYPE_L_P] != ETHTYPE_IP_L_V)
            return 0;
        if (buf[IP_HEADER_LEN_VER_P] != 0x45)
            // must be IP V4 and 20 byte header
            return 0;
        for (byte i = 0; i < 4; ++i)
            if (buf[IP_DST_P+i] !=  ipaddr_[i])
                return 0;
        return 1;
        }

    // make a return eth header from a received eth packet
    static void make_eth(uint8_t *buf)
        {
        //copy the destination mac from the source and fill my mac into src
        for (byte i = 0; i < 6; ++i)
            {
            buf[ETH_DST_MAC + i] = buf[ETH_SRC_MAC + i];
            buf[ETH_SRC_MAC + i] = macaddr_[i];
            }
        }

    // make a new eth header for IP packet
    static void make_eth_ip_new(uint8_t *buf, uint8_t* dst_mac)
        {
        //copy the destination mac from the source and fill my mac into src
        for (byte i = 0; i < 6; ++i)
            {
            buf[ETH_DST_MAC +i]=dst_mac[i];
            buf[ETH_SRC_MAC +i]=macaddr_[i];
            }
                
        buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;
        buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
        }

    static void fill_ip_hdr_checksum(uint8_t *buf)
        {
        uint16_t ck;
        // clear the 2 byte checksum
        buf[IP_CHECKSUM_P]=0;
        buf[IP_CHECKSUM_P+1]=0;
        buf[IP_FLAGS_P]=0x40; // don't fragment
        buf[IP_FLAGS_P+1]=0;  // fragement offset
        buf[IP_TTL_P]=64; // ttl
        // calculate the checksum:
        ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
        buf[IP_CHECKSUM_P]=ck>>8;
        buf[IP_CHECKSUM_P+1]=ck& 0xff;
        }

    // make a new ip header for tcp packet

    // make a return ip header from a received ip packet
    static void make_ip_tcp_new(uint8_t *buf, uint16_t len,uint8_t *dst_ip)
        {
        // set ipv4 and header length
        buf[ IP_P ] = IP_V4_V | IP_HEADER_LENGTH_V;
    
        // set TOS to default 0x00
        buf[ IP_TOS_P ] = 0x00;
    
        // set total length
        buf[ IP_TOTLEN_H_P ] = (len >>8)& 0xff;
        buf[ IP_TOTLEN_L_P ] = len & 0xff;
        
        // set packet identification
        buf[ IP_ID_H_P ] = (ip_identifier_ >>8) & 0xff;
        buf[ IP_ID_L_P ] = ip_identifier_ & 0xff;
        ip_identifier_++;
        
        // set fragment flags   
        buf[ IP_FLAGS_H_P ] = 0x00;
        buf[ IP_FLAGS_L_P ] = 0x00;
        
        // set Time To Live
        buf[ IP_TTL_P ] = 128;
    
        // set ip packettype to tcp/udp/icmp...
        buf[ IP_PROTO_P ] = IP_PROTO_TCP_V;
    
        // set source and destination ip address
        for (byte i = 0; i < 4; ++i)
            {
            buf[IP_DST_P+i]=dst_ip[i];
            buf[IP_SRC_P+i]=ipaddr_[i];
            }
        fill_ip_hdr_checksum(buf);
        }

    // make a return ip header from a received ip packet
    static void make_ip(uint8_t *buf)
        {
        for (byte i = 0; i < 4; ++i)
            {
            buf[IP_DST_P+i]=buf[IP_SRC_P+i];
            buf[IP_SRC_P+i]=ipaddr_[i];
            }
        fill_ip_hdr_checksum(buf);
        }

    // make a return tcp header from a received tcp packet rel_ack_num
    // is how much we must step the seq number received from the other
    // side. We do not send more than 255 bytes of text (=data) in the
    // tcp packet.  If mss=1 then mss is included in the options list
    //
    // After calling this function you can fill in the first data byte
    // at TCP_OPTIONS_P+4. If cp_seq=0 then an initial sequence number
    // is used (should be use in synack) otherwise it is copied from
    // the packet we received
    static void make_tcphead(uint8_t *buf,uint16_t rel_ack_num,uint8_t mss,
			     uint8_t cp_seq)
        {
        uint8_t tseq;

        for (byte i = 0; i < 2; ++i)
            {
            buf[TCP_DST_PORT_H_P+i]=buf[TCP_SRC_PORT_H_P+i];
            buf[TCP_SRC_PORT_H_P+i]=0; // clear source port
            }
        // set source port  (http):
        buf[TCP_SRC_PORT_L_P]=port;

        // sequence numbers:
        // add the rel ack num to SEQACK
        for (byte i = 4; i > 0; --i)
            {
            rel_ack_num=buf[TCP_SEQ_H_P+i-1]+rel_ack_num;
            tseq=buf[TCP_SEQACK_H_P+i-1];
            buf[TCP_SEQACK_H_P+i-1]=0xff&rel_ack_num;
            if (cp_seq)
                {
                // copy the acknum sent to us into the sequence number
                buf[TCP_SEQ_H_P+i-1]=tseq;
                }
            else
                {
                buf[TCP_SEQ_H_P+i-1]= 0; // some preset vallue
                }
            rel_ack_num=rel_ack_num>>8;
            }
        if (cp_seq==0)
            {
            // put inital seq number
            buf[TCP_SEQ_H_P+0]= 0;
            buf[TCP_SEQ_H_P+1]= 0;
            // we step only the second byte, this allows us to send packts 
            // with 255 bytes or 512 (if we step the initial seqnum by 2)
            buf[TCP_SEQ_H_P+2]= seqnum_;
            buf[TCP_SEQ_H_P+3]= 0;
            // step the inititial seq num by something we will not use
            // during this tcp session:
            seqnum_ += 2;
            }
        // zero the checksum
        buf[TCP_CHECKSUM_H_P]=0;
        buf[TCP_CHECKSUM_L_P]=0;

        // The tcp header length is only a 4 bit field (the upper 4 bits).
        // It is calculated in units of 4 bytes. 
        // E.g 24 bytes: 24/4=6 => 0x60=header len field
        //buf[TCP_HEADER_LEN_P]=(((TCP_HEADER_LEN_PLAIN+4)/4)) <<4; // 0x60
        if (mss)
            {
            // the only option we set is MSS to 1408:
            // 1408 in hex is 0x580
            buf[TCP_OPTIONS_P]=2;
            buf[TCP_OPTIONS_P+1]=4;
            buf[TCP_OPTIONS_P+2]=0x05; 
            buf[TCP_OPTIONS_P+3]=0x80;
            // 24 bytes:
            buf[TCP_HEADER_LEN_P]=0x60;
            }
        else
            {
            // no options:
            // 20 bytes:
            buf[TCP_HEADER_LEN_P]=0x50;
            }
        }

    static void make_arp_answer_from_request(uint8_t *buf)
        {
        make_eth(buf);
        buf[ETH_ARP_OPCODE_H_P]=ETH_ARP_OPCODE_REPLY_H_V;
        buf[ETH_ARP_OPCODE_L_P]=ETH_ARP_OPCODE_REPLY_L_V;
        // fill the mac addresses:
        for (byte i = 0; i < 6; ++i)
            {
            buf[ETH_ARP_DST_MAC_P+i]=buf[ETH_ARP_SRC_MAC_P+i];
            buf[ETH_ARP_SRC_MAC_P+i]=macaddr_[i];
            }
        for (byte i=0; i < 4; ++i)
            {
            buf[ETH_ARP_DST_IP_P+i]=buf[ETH_ARP_SRC_IP_P+i];
            buf[ETH_ARP_SRC_IP_P+i]=ipaddr_[i];
            }
        // eth+arp is 42 bytes:
        Ethernet::PacketSend(42,buf); 
        }

    static void make_echo_reply_from_request(uint8_t *buf,uint16_t len)
        {
        make_eth(buf);
        make_ip(buf);
        buf[ICMP_TYPE_P]=ICMP_TYPE_ECHOREPLY_V;
        // we changed only the icmp.type field from request(=8) to reply(=0).
        // we can therefore easily correct the checksum:
        if (buf[ICMP_CHECKSUM_P] > (0xff-0x08))
            buf[ICMP_CHECKSUM_P+1]++;
        buf[ICMP_CHECKSUM_P]+=0x08;
        
        Ethernet::PacketSend(len,buf);
        }

    // you can send a max of 220 bytes of data
    static void make_udp_reply_from_request(uint8_t *buf, char *data,
					    uint8_t datalen, uint16_t dstport)
        {
        uint16_t ck;
        make_eth(buf);
        if (datalen>220)
            datalen=220;
        // total length field in the IP header must be set:
        buf[IP_TOTLEN_H_P]=0;
        buf[IP_TOTLEN_L_P]=IP_HEADER_LEN+UDP_HEADER_LEN+datalen;
        make_ip(buf);
        buf[UDP_DST_PORT_H_P]=dstport>>8;
        buf[UDP_DST_PORT_L_P]=dstport & 0xff;
        // source port does not matter and is what the sender used.
        // calculte the udp length:
        buf[UDP_LEN_H_P]=0;
        buf[UDP_LEN_L_P]=UDP_HEADER_LEN+datalen;
        // zero the checksum
        buf[UDP_CHECKSUM_H_P]=0;
        buf[UDP_CHECKSUM_L_P]=0;
        // copy the data:
        for (byte i = 0; i < datalen; ++i)
            buf[UDP_DATA_P+i]=data[i];
        ck=checksum(&buf[IP_SRC_P], 16 + datalen,1);
        buf[UDP_CHECKSUM_H_P]=ck>>8;
        buf[UDP_CHECKSUM_L_P]=ck& 0xff;
        Ethernet::PacketSend(UDP_HEADER_LEN+IP_HEADER_LEN+ETH_HEADER_LEN+datalen,buf);
        }

    static void make_tcp_synack_from_syn(uint8_t *buf)
        {
        uint16_t ck;
        make_eth(buf);
        // total length field in the IP header must be set:
        // 20 bytes IP + 24 bytes (20tcp+4tcp options)
        buf[IP_TOTLEN_H_P]=0;
        buf[IP_TOTLEN_L_P]=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4;
        make_ip(buf);
        buf[TCP_FLAG_P]=TCP_FLAGS_SYNACK_V;
        make_tcphead(buf,1,1,0);
        // calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + 4 (one option: mss)
        ck=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+4,2);
        buf[TCP_CHECKSUM_H_P]=ck>>8;
        buf[TCP_CHECKSUM_L_P]=ck& 0xff;
        // add 4 for option mss:
        Ethernet::PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4+ETH_HEADER_LEN,
                             buf);
        }

    // get a pointer to the start of tcp data in buf
    // Returns 0 if there is no data
    // You must call init_len_info once before calling this function
    static uint16_t get_tcp_data_pointer(void)
        {
        if (info_data_len_)
            return((uint16_t)TCP_SRC_PORT_H_P+info_hdr_len_);
        else
            return(0);
        }

    // do some basic length calculations and store the result in
    // static varibales
    static void init_len_info(uint8_t *buf)
        {
        info_data_len_ = (buf[IP_TOTLEN_H_P]<<8)|(buf[IP_TOTLEN_L_P]&0xff);
        info_data_len_ -= IP_HEADER_LEN;
        info_hdr_len_ = (buf[TCP_HEADER_LEN_P]>>4)*4; // generate len in bytes;
        info_data_len_ -= info_hdr_len_;
        if (info_data_len_ <= 0)
            info_data_len_ = 0;
        }

    // fill in tcp data at position pos. pos=0 means start of
    // tcp data. Returns the position at which the string after
    // this string could be filled.
    static uint16_t fill_tcp_data_p(uint8_t *buf, uint16_t pos,
                             const char *progmem_s)
        {
        char c;
        // fill in tcp data at position pos
        //
        // with no options the data starts after the checksum + 2 more bytes (urgent ptr)
        while ((c = pgm_read_byte(progmem_s++)))
            {
            buf[TCP_CHECKSUM_L_P+3+pos]=c;
            pos++;
            }
        return(pos);
        }

    // fill in tcp data at position pos. pos=0 means start of
    // tcp data. Returns the position at which the string after
    // this string could be filled.
    static uint16_t fill_tcp_data(uint8_t *buf,uint16_t pos, const char *s)
        {
        // fill in tcp data at position pos
        //
        // with no options the data starts after the checksum + 2 more bytes (urgent ptr)
        while (*s)
            {
            buf[TCP_CHECKSUM_L_P+3+pos] = *s++;
            pos++;
            }
        return(pos);
        }

    // Make just an ack packet with no tcp data inside
    // This will modify the eth/ip/tcp header 
    static void make_tcp_ack_from_any(uint8_t *buf)
        {
        uint16_t j;

        make_eth(buf);
        // fill the header:
        buf[TCP_FLAG_P]=TCP_FLAG_ACK_V;

        if (info_data_len_ == 0)
            // if there is no data then we must still acknoledge one packet
            make_tcphead(buf, 1, 0, 1); // no options
        else
            make_tcphead(buf, info_data_len_, 0, 1); // no options

        // total length field in the IP header must be set:
        // 20 bytes IP + 20 bytes tcp (when no options) 
        j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN;
        buf[IP_TOTLEN_H_P]=j>>8;
        buf[IP_TOTLEN_L_P]=j& 0xff;
        make_ip(buf);
        // calculate the checksum, len=8 (start from ip.src) +
        // TCP_HEADER_LEN_PLAIN + data len
        j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN,2);
        buf[TCP_CHECKSUM_H_P]=j>>8;
        buf[TCP_CHECKSUM_L_P]=j& 0xff;
        Ethernet::PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+ETH_HEADER_LEN,buf);
        }

    // you must have called init_len_info at some time before calling
    // this function dlen is the amount of tcp data (http data) we
    // send in this packet You can use this function only immediately
    // after make_tcp_ack_from_any This is because this function will
    // NOT modify the eth/ip/tcp header except for length and checksum
    static void make_tcp_ack_with_data(uint8_t *buf,uint16_t dlen)
        {
        uint16_t j;
        // fill the header:
        // This code requires that we send only one data packet
        // because we keep no state information. We must therefore set
        // the fin here:
        buf[TCP_FLAG_P]=TCP_FLAG_ACK_V|TCP_FLAG_PUSH_V|TCP_FLAG_FIN_V;
    
        // total length field in the IP header must be set:
        // 20 bytes IP + 20 bytes tcp (when no options) + len of data
        j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen;
        buf[IP_TOTLEN_H_P]=j>>8;
        buf[IP_TOTLEN_L_P]=j& 0xff;
        fill_ip_hdr_checksum(buf);
        // zero the checksum
        buf[TCP_CHECKSUM_H_P]=0;
        buf[TCP_CHECKSUM_L_P]=0;
        // calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
        j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+dlen,2);
        buf[TCP_CHECKSUM_H_P]=j>>8;
        buf[TCP_CHECKSUM_L_P]=j& 0xff;
        Ethernet::PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen+ETH_HEADER_LEN,buf);
        }

    /* new functions for web client interface */
    static void make_arp_request(uint8_t *buf, uint8_t *server_ip)
        {
        for (byte i = 0; i < 6; ++i)
            {
            buf[ETH_DST_MAC +i]=0xff;
            buf[ETH_SRC_MAC +i]=macaddr_[i];
            }
    
        buf[ ETH_TYPE_H_P ] = ETHTYPE_ARP_H_V;
        buf[ ETH_TYPE_L_P ] = ETHTYPE_ARP_L_V;
    
        // generate arp packet
        buf[ARP_OPCODE_H_P]=ARP_OPCODE_REQUEST_H_V;
        buf[ARP_OPCODE_L_P]=ARP_OPCODE_REQUEST_L_V;

        // fill in arp request packet
        // setup hardware type to ethernet 0x0001
        buf[ ARP_HARDWARE_TYPE_H_P ] = ARP_HARDWARE_TYPE_H_V;
        buf[ ARP_HARDWARE_TYPE_L_P ] = ARP_HARDWARE_TYPE_L_V;

        // setup protocol type to ip 0x0800
        buf[ ARP_PROTOCOL_H_P ] = ARP_PROTOCOL_H_V;
        buf[ ARP_PROTOCOL_L_P ] = ARP_PROTOCOL_L_V;

        // setup hardware length to 0x06
        buf[ ARP_HARDWARE_SIZE_P ] = ARP_HARDWARE_SIZE_V;
    
        // setup protocol length to 0x04
        buf[ ARP_PROTOCOL_SIZE_P ] = ARP_PROTOCOL_SIZE_V;

        // setup arp destination and source mac address
        for (byte i = 0; i < 6; ++i)
            {
            buf[ ARP_DST_MAC_P + i ] = 0x00;
            buf[ ARP_SRC_MAC_P + i ] = macaddr_[i];
            }

        // setup arp destination and source ip address
        for (byte i = 0; i < 4; ++i)
            {
            buf[ ARP_DST_IP_P + i ] = server_ip[i];
            buf[ ARP_SRC_IP_P + i ] = ipaddr_[i];
            }

        // eth+arp is 42 bytes:
        Ethernet::PacketSend(42,buf);
        }

    static uint8_t arp_packet_is_myreply_arp ( uint8_t *buf )
        {
        // if packet type is not arp packet exit from function
        if (buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V
            || buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V)
            return 0;
        // check arp request opcode
        if (buf[ARP_OPCODE_H_P] != ARP_OPCODE_REPLY_H_V
            || buf[ARP_OPCODE_L_P] != ARP_OPCODE_REPLY_L_V )
            return 0;
        // if destination ip address in arp packet not match with avr ip address
        for(byte i = 0; i < 4; ++i)
            if(buf[ETH_ARP_DST_IP_P+i] != ipaddr_[i])
                return 0;
        return 1;
        }

    // make a  tcp header
    static void tcp_client_send_packet(uint8_t *buf, uint16_t dest_port,
				       uint16_t src_port, uint8_t flags,
				       uint8_t max_segment_size, 
				       uint8_t clear_seqack,
				       uint16_t next_ack_num, uint16_t dlength,
				       uint8_t *dest_mac, uint8_t *dest_ip)
        {
        uint8_t tseq;
        uint16_t ck;
    
        make_eth_ip_new(buf, dest_mac);

        buf[TCP_DST_PORT_H_P]= (uint8_t) ( (dest_port>>8) & 0xff);
        buf[TCP_DST_PORT_L_P]= (uint8_t) (dest_port & 0xff);

        buf[TCP_SRC_PORT_H_P]= (uint8_t) ( (src_port>>8) & 0xff);
        buf[TCP_SRC_PORT_L_P]= (uint8_t) (src_port & 0xff);

        // sequence numbers:
        // add the rel ack num to SEQACK

        if(next_ack_num)
            for(byte i = 4; i > 0; i--)
                {
                next_ack_num=buf[TCP_SEQ_H_P+i-1]+next_ack_num;
                tseq=buf[TCP_SEQACK_H_P+i-1];
                buf[TCP_SEQACK_H_P+i-1]=0xff&next_ack_num;
                // copy the acknum sent to us into the sequence number
                buf[TCP_SEQ_P + i - 1 ] = tseq;
                next_ack_num>>=8;
                }

        // initial tcp sequence number,require to setup for first
        // transmit/receive
        if(max_segment_size)
            {
            // put inital seq number
            buf[TCP_SEQ_H_P+0]= 0;
            buf[TCP_SEQ_H_P+1]= 0;
            // we step only the second byte, this allows us to send packts
            // with 255 bytes or 512 (if we step the initial seqnum by 2)
            buf[TCP_SEQ_H_P+2]= seqnum_;
            buf[TCP_SEQ_H_P+3]= 0;
            // step the inititial seq num by something we will not use
            // during this tcp session:
            seqnum_ += 2;

            // setup maximum segment size
            buf[TCP_OPTIONS_P]=2;
            buf[TCP_OPTIONS_P+1]=4;
            buf[TCP_OPTIONS_P+2]=0x05;
            buf[TCP_OPTIONS_P+3]=0x80;
            // 24 bytes:
            buf[TCP_HEADER_LEN_P]=0x60;

            dlength +=4;
            }
        else
            {
            // no options:
            // 20 bytes:
            buf[TCP_HEADER_LEN_P]=0x50;
            }
    
        make_ip_tcp_new(buf, IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlength,
                        dest_ip);

        // clear sequence ack number before send tcp SYN packet
        if(clear_seqack)
            {
            buf[TCP_SEQACK_P] = 0;
            buf[TCP_SEQACK_P+1] = 0;
            buf[TCP_SEQACK_P+2] = 0;
            buf[TCP_SEQACK_P+3] = 0;
            }
        // zero the checksum
        buf[TCP_CHECKSUM_H_P]=0;
        buf[TCP_CHECKSUM_L_P]=0;
    
        // set up flags
        buf[TCP_FLAG_P] = flags;
        // setup maximum windows size
        buf[ TCP_WINDOWSIZE_H_P ] = ((600 - IP_HEADER_LEN - ETH_HEADER_LEN)>>8) & 0xff;
        buf[ TCP_WINDOWSIZE_L_P ] = (600 - IP_HEADER_LEN - ETH_HEADER_LEN) & 0xff;

        // setup urgend pointer (not used -> 0)
        buf[ TCP_URGENT_PTR_H_P ] = 0;
        buf[ TCP_URGENT_PTR_L_P ] = 0;
    
        // check sum
        ck=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+dlength,2);
        buf[TCP_CHECKSUM_H_P]=ck>>8;
        buf[TCP_CHECKSUM_L_P]=ck& 0xff;
        // add 4 for option mss:
        Ethernet::PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlength+ETH_HEADER_LEN,buf);
        }

    static uint16_t tcp_get_dlength ( uint8_t *buf )
        {
        int dlength, hlength;
        
        dlength = ( buf[ IP_TOTLEN_H_P ] <<8 ) | ( buf[ IP_TOTLEN_L_P ] );
        dlength -= IP_HEADER_LEN;
        hlength = (buf[ TCP_HEADER_LEN_P ]>>4) * 4; // generate len in bytes;
        dlength -= hlength;
        if ( dlength <= 0 )
            dlength=0;

        return ((uint16_t)dlength);
        }
private:
    static uint16_t ip_identifier_;
    static uint8_t ipaddr_[4];
    static uint8_t macaddr_[6];
    static int16_t info_hdr_len_;
    static int16_t info_data_len_;
    static uint8_t seqnum_;
    };

template <class Ethernet, byte port>
uint16_t IP<Ethernet, port>::ip_identifier_ = 1;
template <class Ethernet, byte port> uint8_t IP<Ethernet, port>::ipaddr_[4];
template <class Ethernet, byte port> uint8_t IP<Ethernet, port>::macaddr_[6];
template <class Ethernet, byte port> int16_t IP<Ethernet, port>::info_hdr_len_;
template <class Ethernet, byte port> int16_t IP<Ethernet, port>::info_data_len_;
template <class Ethernet, byte port> uint8_t IP<Ethernet, port>::seqnum_ = 0xa;

/* end of ip_arp_udp.c */
