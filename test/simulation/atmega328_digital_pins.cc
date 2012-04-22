/* 
   Set all output pins high. 
*/

#include "arduino--.h"

int main(void)
    {
    // Well, this isn't too elegant
    Arduino::D0::modeOutput();
    Arduino::D0::set();

    Arduino::D1::modeOutput();
    Arduino::D1::set();

    Arduino::D2::modeOutput();
    Arduino::D2::set();

    Arduino::D3::modeOutput();
    Arduino::D3::set();

    Arduino::D4::modeOutput();
    Arduino::D4::set();

    Arduino::D5::modeOutput();
    Arduino::D5::set();

    Arduino::D6::modeOutput();
    Arduino::D6::set();

    Arduino::D7::modeOutput();
    Arduino::D7::set();

    Arduino::D8::modeOutput();
    Arduino::D8::set();

    Arduino::D9::modeOutput();
    Arduino::D9::set();

    Arduino::D10::modeOutput();
    Arduino::D10::set();

    Arduino::D11::modeOutput();
    Arduino::D11::set();

    Arduino::D12::modeOutput();
    Arduino::D12::set();

    Arduino::D13::modeOutput();
    Arduino::D13::set();

    return 0;
    }
