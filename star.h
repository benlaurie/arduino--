// -*- mode: c++; indent-tabs-mode: nil; -*-

/*
 * There is one master and up to 256 slaves. Slaves can only send to
 * the master.
 * 
 * The contents are:
 *
 * byte ID;
 * byte TYPE;
 * byte LENGTH;
 * byte MESSAGE[LENGTH];
 *
 * When sent by a slave, ID is the id of the sender, if they have
 * one. When sent by the master, it is the id of the recipient.
 *
 * Request ID (Slave Type 0):
 * 
 *   ID = 0. Note that id 0 is a valid id!
 *
 *   MESSAGE is the MAC of the sender. It can be any length > 0. Two
 *   slaves should not have the same MAC! Note that the master may
 *   impose a limit on MAC length, in which case it will not respond
 *   to a MAC that is too long. A MAC of up to 8 bytes is always
 *   acceptable.
 *
 *   The response is an Allocate ID packet.
 *
 *   This message must be resent periodically until a response is
 *   received. No other message type can be send until an ID is
 *   allocated.
 *
 *
 * Allocate ID (Master Type 0):
 *
 *   ID = allocated id.
 *
 *   MESSAGE is the MAC of the requestor.
 *
 *
 * Out of IDs Error (Master Type 1):
 *
 *   ID = 0
 *
 *   MESSAGE = MAC of the requestor.
 *
 *   Presumably the slave should give some kind of visual indication
 *   if it gets this error. It can retry in case the master changes
 *   its mind.
 *
 */

#include "arduino++.h"
#include "serial.h"  // debug

#include <string.h>

class StarBase
    {
protected:
    enum MessageType
        {
        // Slave
        REQUEST_ID = 0x00,

        // Master
        ALLOCATE_ID = 0x40,
        OUT_OF_IDS = 0x41,
        RESET_ID = 0x42,
        };
    enum MasterType
        {
        };
    static uint32_t protocolError_;
    };

template <class Network, class Observer> class StarNode : public StarBase
    {
protected:
    static void sendPacket(byte id, byte type, byte length, const byte *data)
        {
        Serial.write('P');
        if (!Network::canSend())
            {
            Observer::cantSend();
            return;
            }
        Network::sendPacket(id, type, length, data);
        Serial.write('Z');
        Observer::sentPacket(id, type, length, data);
        Serial.write('W');
        }
    };

template <class Network, class Observer> class StarSlave
  : public StarNode<Network, Observer>
    {
public:
    // Note that this just keeps a pointer, so you need to make sure
    // |mac| stays around.
    static void init(const byte *mac, byte length)
        {
        mac_ = mac;
        length_ = length;
        Network::init();
        }
    static void poll()
        {
        if (Network::dataAvailable())
            processPacket();
        if (!idSet_)
            {
            getID();
            return;
            }
        }
private:
    static void processPacket()
        {
        Serial.write('A');
        switch (Network::getType())
            {
        case StarBase::ALLOCATE_ID:
            Serial.write('C');
            if (Network::getLength() != length_)
                // Can't be us.
                break;
            {
            const byte *data = Network::getData();
            // our ID may have come back
            for (byte n = 0; n < length_; ++n)
                if (data[n] != mac_[n])
                    return;
            }
            id_ = Network::getID();
            idSet_ = true;
            break;

        case StarBase::RESET_ID:
            idSet_ = false;
            break;
            }
        }

    static void getID()
        {
        // try (or retry) to get an ID
        StarNode<Network, Observer>::sendPacket(0, StarBase::REQUEST_ID,
                                                length_, mac_);
        }
    
    static byte id_;
    static bool idSet_;
    static const byte *mac_;
    static byte length_;
    };

template <class Network, class Observer> class StarMaster
  : public StarNode<Network, Observer>
    {
public:
    static void init()
        {
        Network::init();
        resetCount_ = 5;
        }
    static void poll()
        {
        if (Network::dataAvailable())
            processPacket();
        if (resetCount_ > 0)
            { 
            Serial.write('X');
            if (!Network::canSend())
                return;
            Serial.write('Y');
            StarNode<Network, Observer>::sendPacket(0, StarBase::RESET_ID, 0,
                                                    NULL);
            --resetCount_;
            return;
            }
        }
    static void processPacket()
        {
        Observer::gotPacket(Network::getID(), Network::getType(),
                            Network::getLength(), Network::getData());

        switch (Network::getType())
            {
        case StarBase::REQUEST_ID:
            allocateID();
            break;

        default:
            ++StarBase::protocolError_;
            break;
            }
        }
private:
    static void allocateID()
        {
        if (Network::getLength() > Mac::MAX_MAC)
            {
            ++StarBase::protocolError_;
            return;
            }
        byte empty = 0;
        for (byte n = 0; n < NMACS ; )
            {
            if (macs_[n].length() == 0)
                empty = n;
            else if (macs_[n].is(Network::getLength(), Network::getData()))
                {
                macs_[n].sendID(n);
                return;
                }
            if (++n == 0)
                break;
            }
        if (macs_[empty].length() != 0)
            {
            sendPacket(0, StarBase::OUT_OF_IDS, Network::getLength(),
                       Network::getData());
            return;
            }
        macs_[empty].set(Network::getLength(), Network::getData());
        macs_[empty].sendID(empty);
        }

    static const byte NMACS = 8;
    class Mac
        {
    public:
        byte length() const { return length_; }
        bool is(const byte length, const byte *mac)
            { return length == length_ && memcmp(mac, mac_, length) == 0; }
        void set(byte length, const byte *mac)
            {
            length_ = length;
            memcpy(mac_, mac, length_);
            }
        void sendID(byte id)
            {
            StarMaster::sendPacket(id, StarBase::ALLOCATE_ID, length_, mac_);
            Observer::idSent(id, length_, mac_);
            }
        static const byte MAX_MAC = 8;
    private:
        byte length_;
        byte mac_[MAX_MAC];
        };

    static Mac macs_[NMACS];
    static byte resetCount_;
    };


uint32_t StarBase::protocolError_;
template <class Network, class Observer>
  typename StarMaster<Network, Observer>::Mac
  StarMaster<Network, Observer>::macs_[StarMaster<Network, Observer>::NMACS];
template <class Network, class Observer>
  byte StarMaster<Network, Observer>::resetCount_;
template <class Network, class Observer>
  bool StarSlave<Network, Observer>::idSet_;
template <class Network, class Observer> byte StarSlave<Network, Observer>::id_;
template <class Network, class Observer>
  byte StarSlave<Network, Observer>::length_;
template <class Network, class Observer>
  const byte *StarSlave<Network, Observer>::mac_;
