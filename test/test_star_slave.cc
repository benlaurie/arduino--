// -*- mode: c++; indent-tabs-mode: nil; -*-

#include "nanode/mac.h"
#include "serial.h"
#include "rf12star.h"

//static NanodeMAC mac;

int main()
    {
    Nanode::init();
    Serial.begin(57600);
    //Serial.write(mac.ok() ? 'G' : 'B');
    //for (byte n = 0; n < 6; ++n)
    //Serial.writeHex(mac[n]);
    //if (!mac.ok())
    //return 1;

    typedef StarSlave<RF12Star, SerialSlaveObserver> Slave;
    static byte mac[3] = { 1, 2, 3 };
    //slave.setMac(mac, mac.length());
    Slave::init(mac, sizeof mac);

    for ( ; ; )
	Slave::poll();

    return 0;
    }
