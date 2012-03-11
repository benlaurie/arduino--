#include "onewire.h"

static Buttons<Pin::B0> buttons;

template <class Pin>
void Button<Pin>::Dump(_Serial *serial) const
    {
    serial->writeHex(id_, 8);
    serial->write(':');
    serial->writeHex(temperature_);

    short temp = temperature_;
    bool sign = !!(temp & 0x8000);
    if (sign)
	temp = -temp;
    uint32_t t100 = temp * 6 + (temp + 2) / 4;  // * 100 * 1/16 = * 6.25
    serial->write(':');
    if (sign)
	serial->write('-');
    serial->writeDecimal(t100 / 100);
    serial->write('.');
    t100 %= 100;
    serial->writeDecimal(t100, 2);

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

