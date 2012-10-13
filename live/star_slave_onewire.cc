// -*- mode: c++; indent-tabs-mode: nil; -*-

#include "onewire.h"
#include "rf12star.h"

class MySlaveObserver : public NullSlaveObserver
    {
public:
    static void canSend();

    static bool getReadings_;
    };

bool MySlaveObserver::getReadings_;

typedef StarSlave<RF12Star, MySlaveObserver> Slave;
static Buttons<Pin::B0> buttons;
typedef Pin::D6 LED;

void MySlaveObserver::canSend()
    {
    if (!getReadings_)
        return;

    LED::set();
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
    LED::clear();
    getReadings_ = false;
    }

int main()
    {
    Arduino::init();

    LED::clear();
    LED::modeOutput();

    buttons.Init();
    buttons.Scan();

    Slave::init(buttons[0].ID(), 8);

    MySlaveObserver::getReadings_ = true;
    for ( ; ; )
        {
        LED::set();
	Slave::poll();
        LED::clear();
        if (Slave::fastPollNeeded())
            Clock16::sleep(10);
        else
            {
            Clock16::sleep(60000);
            MySlaveObserver::getReadings_ = true;
            }
        }

    return 0;
    }
