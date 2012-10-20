// -*- mode: c++; indent-tabs-mode: nil; -*-
#include "rf12base.h"
#include "star.h"

typedef _RF12Base<Pin::D2, Pin::B2> RF12B;

class RF12Star : RF12B  // not public, we want to hide it
    {
public:
    static void init()
        {
        // Disable other SPI devices on the Nanode
        // FIXME: framework should take care of this
        // ENC28J60
        Pin::B0::set();
        Pin::B0::modeOutput();
        // 23K256
        Pin::B1::set();
        Pin::B1::modeOutput();
        RF12B::init(MHZ868, true);
        enableReceive();
        }
    // If data is available, it should not change until
    // receiveEnable() has been called. This can be called multiple
    // times safely (in contrast to standard Arduino RF12B libraries).
    static bool dataAvailable()
        {
        if (recvDoneNoEnable())
            {
            if (goodCRC())
                return true;
            enableReceive();
            }
        return false;
        }
    // Once this is called, received data can change. Receive is only
    // enabled if transmit is not in progress.
    static void enableReceive()
        { RF12B::enableReceive(); }
    static byte getID()
        { return data()[0]; }
    // Retrieve the type field from the current received packet.
    static byte getType()
        { return header(); }
    // Retrieve the length of the data in the current received packet.
    static byte getLength()
        { return length() - 1; }
    // Retrieve a copy of the received data. This must be constant
    // until dataAvailable() is called again.
    static const byte *getData()
        { return data() + 1; }
    // Can we send a packet?
    static bool canSend()
        { return RF12B::canSend(); }
    // Do we need a fast poll (true until we've finished sending)
    static bool fastPollNeeded()
        { return !canSend(); }
    // Send a packet. Only call if canSend() returns true.
    static void sendPacket(byte id, byte type, byte length, const byte *data)
        {
        setHeader(type);
        clearData();
        writeData(&id, 1);
        writeData(data, length);
        sendStart();
        }
    };

// FIXME: this should be in rf12base.
SIGNAL(INT0_vect)
    {
    RF12B::interrupt();
    }
