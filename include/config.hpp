#pragma once

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include <array>

constexpr uint8_t MAX_VALUE_CHARS = 11;
typedef std::array<char, MAX_VALUE_CHARS> string_value_t;

namespace buttonConfig
{
    constexpr gpio_num_t pin = GPIO_NUM_7;
    constexpr uint32_t shortPressTimeMs = 50;
    constexpr int activeLevel = 0;
}

namespace uartConfig
{
    constexpr gpio_num_t rxPin = GPIO_NUM_16;
    constexpr gpio_num_t txPin = GPIO_NUM_17;
}

namespace i2cConfig
{
    constexpr i2c_port_t hostPort = I2C_NUM_0;
    constexpr gpio_num_t sdaPin = GPIO_NUM_6;
    constexpr gpio_num_t sclPin = GPIO_NUM_5;
    constexpr gpio_num_t rstPin = GPIO_NUM_NC;
    constexpr uint32_t clockSpeedHz = 400 * 1000;
}

namespace ledConfig
{
    constexpr uint8_t maxColorValue = 255;
}

namespace lcdConfig
{  
    constexpr uint8_t i2cAddress = 0x3C;
    constexpr uint8_t hRes = 128;
    constexpr uint8_t vRes = 64;
    constexpr uint8_t cmdBits = 8;
    constexpr uint8_t paramBits = 8;
    constexpr uint8_t maxBarHeight = 50;
    constexpr uint8_t initialBarHeight = 25;
}

namespace meteoConfig
{
    constexpr uint16_t refreshIntervalMs = 1000;
}