// -*- mode: c++; indent-tabs-mode: nil; -*-

// If we used Timer1 for the clock, we could scale it even slower.
#define CLOCK16_PRESCALE 1024

#include "onewire.h"
#include "rf12star.h"

#define READING_FREQUENCY 60000

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
    Nanode::init();
    Timer0::prescaler1024();

    LED::clear();
    LED::modeOutput();

    buttons.Init();
    buttons.Scan();

    Slave::init(buttons[0].ID(), 8);

    MySlaveObserver::getReadings_ = true;
    uint16_t lastReading = Clock16::millis();

    // FIXME: put this in a library somewhere
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
#ifdef sleep_bod_disable
    sleep_bod_disable();
#endif
    for ( ; ; )
        {
        LED::set();
	Slave::poll();
        LED::clear();

        // Sleep until something happens
        sei();
        sleep_cpu();

        if (Clock16::millis() - lastReading > READING_FREQUENCY)
            {
            lastReading = Clock16::millis();
            MySlaveObserver::getReadings_ = true;
            }
        }

    return 0;
    }
