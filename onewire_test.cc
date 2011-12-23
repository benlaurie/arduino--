#include "onewire.h"

static Buttons<Pin::C3> buttons;

void setup()
    {
    // initialize DS18B20 datapin
    buttons.Init();
    }

int main()
    {
    setup();

    for ( ; ; )
	{
	buttons.Scan();
	buttons.GetTemperatures();
	buttons.GetParasites();
	}
    return 0;
    }

