#include "nanode/mac.h"
#include "serial.h"

static NanodeMAC mac;

int main()
    {
    Arduino::init();
    Serial.begin(57600);
    for (byte n = 0; n < 6; ++n)
	Serial.writeHex(mac[n]);

    return 0;
    }
