// -*- mode: c++; indent-tabs-mode: nil; -*-

#include "nanode/mac.h"
#include "serial.h"
#include "rf12star.h"

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

//static NanodeMAC mac;

int main()
    {
    Arduino::init();
    Serial.begin(57600);
    //Serial.write(mac.ok() ? 'G' : 'B');
    //for (byte n = 0; n < 6; ++n)
    //Serial.writeHex(mac[n]);
    //if (!mac.ok())
    //return 1;

    StarSlave<RF12Star, SerialObserver> slave;
    static byte mac[3] = { 1, 2, 3 };
    //slave.setMac(mac, mac.length());
    slave.init(mac, sizeof mac);

    for ( ; ; )
	slave.poll();

    return 0;
    }
