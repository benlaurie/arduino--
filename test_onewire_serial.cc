#include "onewire.h"

static Buttons<Pin::B0> buttons;

template <class Pin>
void Button<Pin>::Dump(_Serial *serial) const
    {
    serial->writeHex(id_, 8);
    serial->write(':');
    serial->writeHex(temperature_);
    serial->write('\r');
    serial->write('\n');
    }

int main()
    {
    // Without this it sends but does not receive.
    Arduino::init();
    Serial.begin(57600);

    Serial.write("Test start\r\n");

    buttons.Init();
    for ( ; ; )
	{
	buttons.Scan();
	buttons.GetTemperatures();
	buttons.GetParasites();
	buttons.Dump(&Serial);
	}
    return 0;
    }

