#pragma once

#include <stdint.h>
#include "driver/gpio.h"
#include "led_strip.h"

namespace led
{
    enum class Color {
        Red,
        Green,
        Blue,
    };

    class LedHandler
    {
    public:
        void init();
        void setColor(Color color, uint8_t value);
        void turnOff();
        void turnOn();

    private:
        uint8_t redValue_ = 127;
        uint8_t greenValue_ = 127;
        uint8_t blueValue_ = 127;
        bool isOn_ = false;

        static constexpr gpio_num_t BUILT_IN_LED_PIN = GPIO_NUM_48;
        led_strip_handle_t ledStrip_ = NULL;
    };
}