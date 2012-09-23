#include "arduino--.h"
#include "clock16.h"

int main(void)
    {
    Arduino::init();
    Arduino::D6::modeOutput();
    while(true)
        {
        Arduino::D6::toggle();
        Clock16::sleep(500);
        }
    return 0;
    }
