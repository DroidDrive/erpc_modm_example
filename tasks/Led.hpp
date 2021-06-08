#pragma once

#include <modm/board.hpp>
#include <modm/processing/protothread.hpp>

using Led1 = Board::LedGreen;
using Led2 = Board::LedBlue;


class BlinkingLight : public modm::pt::Protothread
{
public:
    bool
    run()
    {
        PT_BEGIN();
        Led1::setOutput();
        Led2::setOutput();
        while (true){
            timeout.restart(100ms);
            PT_WAIT_UNTIL(timeout.isExpired());
            Led1::toggle();
            Led2::toggle();
        }
        PT_END();
    }

private:
    modm::ShortTimeout timeout;
};

