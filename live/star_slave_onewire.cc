// -*- mode: c++; indent-tabs-mode: nil; -*-

#include "onewire.h"
#include "rf12star.h"

class MySlaveObserver : public NullSlaveObserver
    {
public:
    static void canSend();
    };

typedef StarSlave<RF12Star, MySlaveObserver> Slave;
static Buttons<Pin::B0> buttons;

void MySlaveObserver::canSend()
    {
    buttons.GetTemperatures();
    byte buf[60];

    byte n;
    for (n = 0; n < 6 && n < buttons.Count(); ++n)
	{
	memcpy(&buf[n*10], buttons[n].ID(), 8);
	buf[n*10 + 8] = buttons[n].Temperature() & 0xff;
	buf[n*10 + 9] = buttons[n].Temperature() >> 8;
	}
    Slave::sendPacket(0, n * 10, buf);
    }

int main()
    {
    Arduino::init();

    buttons.Init();
    buttons.Scan();

    Slave::init(buttons[0].ID(), 8);

    for ( ; ; )
	Slave::poll();

    return 0;
    }
