#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

#include "arduino++.h"

template<class Pin, int debounce, typename int_t>
class PushButton
    {
public:

    enum event_type 
        {
        none,
        keyup,
        keydown
        };

    void init()
        {
        Pin::modeInput();
        // aktivate pullup
        Pin::set();
        previous_ = !Pin::read();
        changed_ = static_cast<int_t>(Arduino::millis());
        duration_ = 0;
        }

    event_type read()
        {
        const int_t now = static_cast<int_t>(Arduino::millis());
        // The button is active low
        const bool pressed = !Pin::read();

        // If the switch changed, due to noise or pressing...
        if (pressed != previous_) 
            {
            // reset the debouncing timer
            previous_ = pressed;
            const int_t delta = now - changed_;
            changed_ = now;
    
            if (delta > debounce) 
                {
                if (pressed)
                    {
                    duration_ = now;
                    return keydown;
                    }
            
                duration_ = now - duration_;
                return keyup;
                }
            }
        return none;
        }

    // duration is only valid after a keyup event
    int_t duration() { return duration_; }

private:
    bool previous_;
    int_t changed_;
    int_t duration_;
};

#endif
