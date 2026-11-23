#include "led.hpp"
#include "led_strip.h"

namespace led
{
    void LedHandler::init()
    {
        led_strip_config_t strip_config = {};
        strip_config.strip_gpio_num = BUILT_IN_LED_PIN;
        strip_config.max_leds = 1;
        strip_config.led_model = LED_MODEL_WS2812;
        strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
        strip_config.flags.invert_out = false;

        led_strip_rmt_config_t rmt_config = {};
        rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;
        rmt_config.resolution_hz = 10 * 1000 * 1000;
        rmt_config.mem_block_symbols = 64;
        rmt_config.flags.with_dma = false;

        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &ledStrip_));
        turnOff();
    }

    void LedHandler::setColor(Color color, uint8_t value)
    {
        switch (color)
        {
        case Color::Red:
            redValue_ = value;
            break;
        case Color::Green:
            greenValue_ = value;
            break;
        case Color::Blue:
            blueValue_ = value;
        default:
            break;
        }

        if (isOn_)
        {
            turnOn();
        }
    }

    void LedHandler::turnOn()
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(ledStrip_, 0, redValue_, greenValue_, blueValue_));
        ESP_ERROR_CHECK(led_strip_refresh(ledStrip_));
        isOn_ = true;
    }

    void LedHandler::turnOff()
    {
        ESP_ERROR_CHECK(led_strip_clear(ledStrip_));
        isOn_ = false;
    }
}