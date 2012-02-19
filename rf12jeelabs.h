// -*- mode: c++; indent-tabs-mode: nil; -*-
// 2009-02-09 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// 2011-12-28 Ben Laurie <ben@links.org>

// Jeelabs RF12B driver compatible upper layer.

/*
 * Packets look like:
 *
 * SYNC1 SYNC2 HDR LEN ... CRC1 CRC2
 *
 * SYNC1 is 2d
 * SYNC2 is the group, which is d4 by default
 * HDR   is
 *          bit 7 CTL
 *          bit 6 DST
 *          bit 5 ACK
 *          bit 4-0 ID
 * LEN   is the length of the data
 * ...   is the data
 * CRC   is a CRC over SYNC2 onwards
 *
 * Nodes can only send to other nodes in the same group (nominally you
 * can send outside the group, but the chip recognises the group code,
 * so you can only receive messages with your group number on them).
 *
 * If DST=1, then ID is the id of the destination, otherwise it is the
 * ID of the source.
 *
 * A packet with ACK=1 and CTL=0 wants an ack.
 * An ack depends on DST in the received packet
 *           DST=1 -> CTL=1 DST=0 ACK=0 ID=sender ID (== destination
 *                                                    ID of original packet)
 *           DST=0 -> CTL=1 DST=1 ACK=0 ID=source ID of original packet
 *
 * It is not clear how the recipient, in the first case, knows the ack
 * is theirs?
 *
 * A packet with CTL=1 is an ack. If DST=1, then ID is the id of the
 * node the ack is aimed at.
 *        
 */

#include "rf12base.h"

#define RF12_HDR_CTL    0x80
#define RF12_HDR_DST    0x40
#define RF12_HDR_ACK    0x20
#define RF12_HDR_MASK   0x1F

template <class RFM_IRQ> class RF12BJeelabs : public _RF12Base<RFM_IRQ>
    {
    static uint8_t _nodeid;              // address of this node
    static long _seq;             // seq number of encrypted packet (or -1)

    static uint32_t _seqNum;             // encrypted send sequence number
    static uint32_t _cryptKey[4];        // encryption key to use
    static void (*_crypter)(uint8_t);    // does en-/decryption (null
                                         // if disabled)

public:
    // call this once with the node ID, frequency band, and optional group
    static void init(uint8_t id, uint8_t band, uint8_t group = 0xD4)
        {
        _nodeid = id;
	_RF12Base<RFM_IRQ>::init(band, (_nodeid & NODE_ID) != 0, group);
	}

    static byte header() { return _RF12Base<RFM_IRQ>::header(); }
    static void setHeader(byte hdr) { _RF12Base<RFM_IRQ>::setHeader(hdr); }
    static bool goodCRC() { return _RF12Base<RFM_IRQ>::goodCRC(); }

    // call this frequently, returns true if a packet has been received
    static bool recvDone(void)
        {
	if (!_RF12Base<RFM_IRQ>::recvDone())
	    return false;
	if (!(header() & RF12_HDR_DST) || (_nodeid & NODE_ID) == 31 ||
	    (header() & RF12_HDR_MASK) == (_nodeid & NODE_ID))
	    {
	    if (goodCRC() && _crypter != 0)
		_crypter(0);
	    else
		_seq = -1;
	    return true; // it's a broadcast packet or it's addressed
                          // to this node
	    }
        return false;
        }

    // returns true if the buffer currently contains a packet that
    // needs ACKing.
    static bool wantsAck()
        {
        return (header() & RF12_HDR_ACK) && !(header() & RF12_HDR_CTL);
        }

    // Send an ack reply to the packet in the buffer (wantsAck() must be true)
    static void sendAckReply()
        {
        byte hdr;

        if (header() & RF12_HDR_DST)
            hdr = RF12_HDR_CTL;
        else
            hdr = RF12_HDR_CTL | RF12_HDR_DST | (header() & RF12_HDR_MASK);
        sendStart(hdr, 0, 0);
        }
    static bool isAckReply() { return (header() & RF12_HDR_CTL) != 0; }

    // call this only when rf12_recvDone() or rf12_canSend() return true
    static void sendStart(uint8_t hdr)
        {
	sendStart(hdr, NULL, 0);
        }

    static void sendStart(uint8_t hdr, const void* ptr, uint8_t len)
        {
        setHeader(hdr & RF12_HDR_DST ? hdr :
		  (hdr & ~RF12_HDR_MASK) + (_nodeid & NODE_ID));
        if (_crypter != 0)
            _crypter(1);
        _RF12Base<RFM_IRQ>::sendStart(ptr, len);
        }
    };

template <class RFM_IRQ> byte RF12BJeelabs<RFM_IRQ>::_nodeid;
template <class RFM_IRQ> long RF12BJeelabs<RFM_IRQ>::_seq;
template <class RFM_IRQ> void (*RF12BJeelabs<RFM_IRQ>::_crypter)(byte);

// Setup for Jeenodes and Wi/Nanodes.
typedef RF12BJeelabs<Pin::D2> RF12B;

SIGNAL(INT0_vect)
    {
    RF12B::interrupt();
    }
