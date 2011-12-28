#include "rf12.h"
#include "serial.h"

// You need to set these the other way round for the second test node.
static const byte id = 2;
static const byte dest = 1;

int main()
    {
    unsigned long last = 0;

    Arduino::init();
    Serial.begin(57600);
    RF12B::init(id, RF12B::MHZ868);
    for ( ; ; )
	{
	unsigned long t = Arduino::millis();
	if (t > last + 100 && RF12B::canSend())
	    {
	    last = t;
	    char buf[1];
	    buf[0] = id;
	    RF12B::sendStart(RF12_HDR_ACK | RF12_HDR_DST | dest, buf,
			     sizeof buf);
	    Serial.write('s');
	    }

	if (RF12B::recvDone())
	    {
	    if (!RF12B::goodCRC())
		Serial.write('?');
	    else
		{	
		Serial.write('r');
		Serial.write(RF12B::length() + '0');
		Serial.write(RF12B::data()[0] + '0');
		if (RF12B::wantsAck())
		    {
		    RF12B::sendAckReply();
		    Serial.write('a');
		    }
		}
	    }
	}
    }
