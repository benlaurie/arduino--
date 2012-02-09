// -*- mode: c++; indent-tabs-mode: nil; -*-

#include "nanode/mac.h"
#include "serial.h"

static NanodeMAC mac;

int main()
    {
    Arduino::init();
    Serial.begin(57600);
    Serial.write(mac.ok() ? 'G' : 'B');
    for (byte n = 0; n < 6; ++n)
        Serial.writeHex(mac[n]);
    if (!mac.ok())
        return 1;

    for ( ; ; )
        {
        NanodeMAC mac2;

        Serial.write(mac2.ok() ? 'G' : 'B');
        for (byte n = 0; n < 6; ++n)
            Serial.writeHex(mac2[n]);
        if (!mac2.ok())
            return 1;
        }

    return 0;
    }
