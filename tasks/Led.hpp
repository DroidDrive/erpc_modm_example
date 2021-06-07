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

        // set everything up
        Led1::setOutput();
        Led2::setOutput();
        Led1::set();
        Led2::set();
        while (true)
        {
            timeout.restart(100ms);
            Led1::set();
            Led2::reset();
            PT_WAIT_UNTIL(timeout.isExpired());

            timeout.restart(200ms);
            Led1::reset();
            Led2::set();
            PT_WAIT_UNTIL(timeout.isExpired());
        }

        PT_END();
    }

private:
    modm::ShortTimeout timeout;
};

