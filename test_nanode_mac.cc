// -*- mode: c++; indent-tabs-mode: nil; -*-

#include "nanode/mac.h"
#include "serial.h"
// FIXME: leave this out and we get a hang after 7 characters ... why?
#include "clock32.h"

static NanodeMAC mac;

int main()
    {
    Arduino::init();
    Serial.begin(57600);
    Serial.write(mac.ok() ? 'G' : 'B');
    for (byte n = 0; n < 6; ++n)
        Serial.writeHex(mac[n]);

    return 0;
    }
