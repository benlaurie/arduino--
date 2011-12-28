#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

#include "arduino++.h"

template<class Arduino_, class Pin, int debounce>
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
        changed_ = Arduino_::millis();
        duration_ = 0;
        }

    event_type read()
        {
        const typename Arduino_::time_res_t now = Arduino_::millis();
        // The button is active low
        const bool pressed = !Pin::read();

        // If the switch changed, due to noise or pressing...
        if (pressed != previous_) 
            {
            // reset the debouncing timer
            previous_ = pressed;
            const typename Arduino_::time_res_t delta = now - changed_;
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
    typename Arduino_::time_res_t duration() { return duration_; }

private:
    bool previous_;
<<<<<<< HEAD
    typename Arduino_::time_res_t changed_;
    typename Arduino_::time_res_t duration_;
=======
    int_t changed_;
    int_t duration_;
>>>>>>> master
};

#endif
