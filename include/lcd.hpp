#pragma once

#include "driver/i2c_master.h"
#include <ssd1306.h>
#include "esp_lvgl_port.h"
#include "config.hpp"

namespace lcd
{
    struct ColorBar
    {
        lv_obj_t *rect = nullptr;
        lv_obj_t *valueLabel = nullptr;
        uint8_t width = 10;
        uint8_t maxHeight = lcdConfig::maxBarHeight;
        uint8_t posX = 0;
        uint8_t posY = 0;
        const char *label = nullptr;
    };

    enum class Color
    {
        Red,
        Green,
        Blue,
        Unknown,
    };

    enum class MeteoUnit {
        Temperature,
        Humidity,
        Pressure,
    };

    struct MeteoItem
    {
        lv_obj_t *valueLabel = nullptr;
        uint8_t posX = 0;
        uint8_t posY = 0;
        const char *label = nullptr;
    };

    class LcdHandler
    {
    public:
        void init(i2c_master_bus_handle_t masterBus);
        void initBar(ColorBar &bar);
        void initMeteoItem(MeteoItem &meteoItem);
        void updateBarData(Color color, uint8_t height, uint8_t value);
        void updateMeteoItem(MeteoUnit unit, const char* stringValue);

    private:
        lv_disp_t *disp_ = nullptr;
        i2c_master_bus_handle_t i2cBus_ = nullptr;
        lv_obj_t *rect_ = nullptr;

        ColorBar redBar_ = {.posX = 20, .posY = 5, .label = "R"};
        ColorBar greenBar_ = {.posX = 65, .posY = 5, .label = "G"};
        ColorBar blueBar_ = {.posX = 110, .posY = 5, .label = "B"};

        MeteoItem temperature_ = {.posX = 0, .posY = 0, .label = "T:"};
        MeteoItem humidity_ = {.posX = 0, .posY = 22, .label = "RH:"};
        MeteoItem pressure_ = {.posX = 0, .posY = 44, .label = "P:"};
    };
}