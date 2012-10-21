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

#include "arduino--.h"

#include <string.h>

class StarBase
    {
public:
    enum MessageType
        {
        // Slave
        REQUEST_ID = 0x00,
        USER_SLAVE_MESSAGE = 0x80,

        // Master
        ALLOCATE_ID = 0x40,
        OUT_OF_IDS = 0x41,
        RESET_ID = 0x42,
        USER_MASTER_MESSAGE = 0xc0,
        };
    enum MasterType
        {
        };
protected:
    static uint32_t protocolError_;
    };

template <class Network, class Observer> class StarNode : public StarBase
    {
protected:
    static void sendPacket(byte id, byte type, byte length, const byte *data)
        {
        if (!Network::canSend())
            {
            Observer::cantSend();
            return;
            }
        Network::sendPacket(id, type, length, data);
        Observer::sentPacket(id, type, length, data);
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
    static bool pollNeeded()
        {
        if (Network::dataAvailable()
            || (Network::canSend() && (Observer::wantSend()
                                       || !idSet_)))
            return true;

        Network::enableReceive();
        return false;
        }
    static void poll()
        {
        if (Network::dataAvailable())
            processPacket();
        if (Network::canSend())
            {
            if (!idSet_)
                getID();
            else
                Observer::canSend();
            }
        Network::enableReceive();
        }
    static bool fastPollNeeded()
        { return Network::fastPollNeeded(); }
    // Only call this from Observer::canSend()
    static void sendPacket(byte type, byte length, const byte *data)
        {
        StarNode<Network, Observer>::sendPacket
            (id_, type | StarBase::USER_SLAVE_MESSAGE, length, data);
        }
private:
    static void processPacket()
        {
        Observer::gotPacket(Network::getID(), Network::getType(),
                            Network::getLength(), Network::getData());
        switch (Network::getType())
            {
        case StarBase::ALLOCATE_ID:
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

        default:
            ++StarBase::protocolError_;
            Observer::protocolError(Network::getID(), Network::getType(),
                                    Network::getLength(), Network::getData());
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

template <class Network, class Observer, class Processor> class StarMaster
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
            if (!Network::canSend())
                {
                Observer::cantSend();
                return;
                }
            StarNode<Network, Observer>::sendPacket(0, StarBase::RESET_ID, 0,
                                                    NULL);
            --resetCount_;
            return;
            }
        }
    static void processPacket()
        {
        byte type = Network::getType();

        Observer::gotPacket(Network::getID(), type, Network::getLength(),
                            Network::getData());

        if (type != StarBase::REQUEST_ID
            && macs_[Network::getID()].length() == 0)
            {
            resetCount_ = 1;
            return;
            }

        switch (type)
            {
        case StarBase::REQUEST_ID:
            allocateID();
            break;

        default:
            if (type >= StarBase::USER_SLAVE_MESSAGE
                && type < StarBase::USER_MASTER_MESSAGE)
                Processor::processUserMessage(type, Network::getLength(),
                                              Network::getData());
            else
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
            StarNode<Network, Observer>::sendPacket(
              0, 
              StarBase::OUT_OF_IDS, Network::getLength(),
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


#define T template <class Network, class Observer, class Processor>
#define M StarMaster<Network, Observer, Processor>

T typename M::Mac M::macs_[M::NMACS];
T byte M::resetCount_;

#undef D
#undef M

template <class Network, class Observer>
  bool StarSlave<Network, Observer>::idSet_;
template <class Network, class Observer> byte StarSlave<Network, Observer>::id_;
template <class Network, class Observer>
  byte StarSlave<Network, Observer>::length_;
template <class Network, class Observer>
  const byte *StarSlave<Network, Observer>::mac_;

class NullSlaveObserver
    {
public:
    static void cantSend() {}
    static void gotPacket(byte id, byte type, byte length, const byte *data)
	{}
    static void protocolError(byte id, byte type, byte length, const byte *data)
	{}
    static void sentPacket(byte id, byte type, byte length, const byte *data)
	{}
    static void canSend() {}
    };

