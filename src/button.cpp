#include "button.hpp"
#include "config.hpp"
#include "iot_button.h"
#include "button_gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

namespace
{
    const char *TAG = "BUTTON";
} // namespace

namespace button
{
    void ButtonHandler::init(gpio_num_t pin, uint32_t shortPressTime, int activeLevel)
    {
        button_config_t btnConfig = {};
        btnConfig.short_press_time = shortPressTime;

        button_gpio_config_t btnGpioConfig = {};

        btnGpioConfig.gpio_num = pin;
        btnGpioConfig.active_level = activeLevel;
        btnGpioConfig.disable_pull = false;

        ESP_ERROR_CHECK(iot_button_new_gpio_device(&btnConfig, &btnGpioConfig, &btn_));
    }

    void ButtonHandler::setOnPressCallback(EventCallback cb)
    {
        ESP_LOGI(TAG, "onButtonPress()");
        if (!btn_)
        {
            return;
        }

        onPressCallback_ = cb;
        ESP_ERROR_CHECK(iot_button_register_cb(btn_, BUTTON_PRESS_DOWN, NULL, onPressCallback_, NULL));
    }

    void ButtonHandler::setOnReleaseCallback(EventCallback cb)
    {
        ESP_LOGI(TAG, "onButtonRelease()");
        if (!btn_)
        {
            return;
        }

        onReleaseCallback_ = cb;
        ESP_ERROR_CHECK(iot_button_register_cb(btn_, BUTTON_PRESS_UP, NULL, onReleaseCallback_, NULL));
    }
} // namespace button
