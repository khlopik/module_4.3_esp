#pragma once

#include "bme280.h"
#include "config.hpp"

namespace
{
    // constexpr uint8_t MAX_VALUE_CHARS = 11;
}

namespace meteo
{
    // typedef std::array<char, MAX_VALUE_CHARS> string_value_t;

    enum class Format
    {
        Integer,
        TwoDecimals,
    };

    enum class Unit : uint8_t
    {
        Degree,
        Percent,
        Mm
    };

    constexpr const char *unitToString(Unit unit)
    {
        switch (unit)
        {
        case Unit::Degree:
            return "°C";
        case Unit::Percent:
            return "%";
        case Unit::Mm:
            return "mm";
        }
        return "";
    }

    using EventCallback = void (*)(const char*);

    class MeteoHandler
    {
    public:
        void init();
        string_value_t getTemperatureString() const;
        string_value_t getHumidityString() const;
        string_value_t getPressureString() const;
        void setCallbackOnTemperatureUpdate(EventCallback cb);
        void setCallbackOnHumidityUpdate(EventCallback cb);
        void setCallbackOnPressureUpdate(EventCallback cb);

    private:
        // static constexpr uint8_t MAX_VALUE_CHARS = 7;
        bme280_handle_t bme280_ = nullptr;
        int32_t tempX100_ = 0;
        int32_t humidX100_ = 0;
        int32_t pressure_ = 0;
        static void updateTask(void *pvParams);
        void updateTemperature();
        void updateHumidity();
        void updatePressure();
        void updateLoop();
        string_value_t convertValueToString(int32_t value, Format format, Unit unit) const;
        EventCallback onTempUpdateCb_ = nullptr;
        EventCallback onHumidUpdateCb_ = nullptr;
        EventCallback onPressUpdateCb_ = nullptr;
    };
}