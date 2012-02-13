#include "onewire.h"

static Buttons<Pin::C3> buttons;

template <class Pin>
void Button<Pin>::Dump(_Serial *serial) const
    {
    serial->writeHex(id_, 8);
    serial->write(':');
    serial->write(temperature_);
    serial->write('\r');
    serial->write('\n');
    }

int main()
    {
    // Without this it sends but does not receive.
    Arduino::init();
    Serial.begin(57600);

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

