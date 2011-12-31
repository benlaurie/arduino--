// -*- mode: c++; indent-tabs-mode: nil; -*-
#include "rf12.h"
#include "serial.h"

// This test hangs randomly without a watchdog timer.
#define WATCHDOG

#ifdef WATCHDOG
# include <avr/wdt.h>
#endif

// You need to set these the other way round for the second test node.
static const byte id = 1;
static const byte dest = 2;

int main()
    {
    unsigned long last = 0;
    byte seq = 0;

    Arduino::init();
    Serial.begin(57600);
    RF12B::init(id, RF12B::MHZ868);
#ifdef WATCHDOG
    wdt_enable(WDTO_2S);
#endif
    for ( ; ; )
        {
#ifdef WATCHDOG
        wdt_reset();
#endif
        unsigned long t = Arduino::millis();
        if (t > last + 100 && RF12B::canSend())
            {
            last = t;
            char buf[2];
            buf[0] = id;
            buf[1] = ++seq;
            RF12B::sendStart(RF12_HDR_ACK | RF12_HDR_DST | dest, buf,
                             sizeof buf);
            Serial.write('s');
            Serial.writeHex(RF12B::header());
            }

        if (RF12B::recvDone())
            {
            if (!RF12B::goodCRC())
                Serial.write('?');
            else
                {       
                Serial.write('r');
                Serial.writeHex(RF12B::header());
                Serial.write(RF12B::length() + '0');
                if (RF12B::isAckReply())
                    Serial.write('A');
                else
                    {
                    Serial.write(RF12B::data()[0] + '0');
                    Serial.writeHex(RF12B::data()[1]);
                    if (RF12B::wantsAck())
                        {
                        RF12B::sendAckReply();
                        Serial.write('a');
                        }
                    }
                }
            }
        }
    }
