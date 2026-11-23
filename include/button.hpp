#pragma once

#include "freertos/FreeRTOS.h"
#include "button_gpio.h"
#include "driver/gpio.h"

namespace button
{
    using EventCallback = void (*)(void *arg, void *data);

    class ButtonHandler
    {
    public:
        void init(gpio_num_t pin, uint32_t shortPressTime, int activeLevel);
        void setOnPressCallback(EventCallback cb);
        void setOnReleaseCallback(EventCallback cb);

    private:
        button_handle_t btn_ = nullptr;
        EventCallback onPressCallback_ = nullptr;
        EventCallback onReleaseCallback_ = nullptr;
    };
}