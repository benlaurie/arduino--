// -*- mode: c++; indent-tabs-mode: nil; -*-
#include "rf12.h"
#include "clock16.h"
#include "serial.h"

// This test hangs randomly without a watchdog timer on a Nanode.
//#define WATCHDOG

#ifdef WATCHDOG
# include <avr/wdt.h>

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

void get_mcusr(void) 
  __attribute__((naked))
  __attribute__((section(".init3")));
void get_mcusr(void)
    {
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
    }
#endif

// You need to set these the other way round for the second test node.
static const byte id = 2;
static const byte dest = 1;

int main()
    {
    Clock16::time_res_t last = 0;
    byte seq = 0;

    Arduino::init();

    // Disable other SPI devices on the Nanode
    // FIXME: framework should take care of this
    // ENC28J60
    Pin::B0::set();
    Pin::B0::modeOutput();
    // 23K256
    Pin::B1::set();
    Pin::B1::modeOutput();

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
        Clock16::time_res_t t = Clock16::millis();

        if (t - last > 100 && RF12B::canSend())
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
