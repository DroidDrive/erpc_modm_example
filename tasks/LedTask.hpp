#pragma once

#include <modm/board.hpp>

#include <utils/Task.hpp>

template <class Gpio, int SleepTime>
class LedTask : public ModmTask{
public:
    LedTask() : ModmTask("LedTask", 6)
    {}
    void task()
    {
        Gpio::setOutput();
        while (true)
        {
            sleep(SleepTime * MILLISECONDS);
            Gpio::toggle();
            {
                static modm::rtos::Mutex lm;
                modm::rtos::MutexGuard m(lm);
            }
        }
    }
};