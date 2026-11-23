#include "meteo.hpp"
#include "i2c_bus.h"
#include "bme280.h"
#include "config.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <cmath>
#include <array>
#include <algorithm>

namespace
{
    const char *TAG = "METEO";
} // namespace

namespace meteo
{
    void MeteoHandler::init()
    {
        ESP_LOGI(TAG, "== init() ==");
        static i2c_bus_handle_t i2c_bus = NULL;

        // Step1: Init I2C bus
        i2c_config_t conf = {};
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = i2cConfig::sdaPin;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_io_num = i2cConfig::sclPin;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = i2cConfig::clockSpeedHz;

        i2c_bus = i2c_bus_create(i2cConfig::hostPort, &conf);

        uint8_t found[16] = {};
        uint8_t count = i2c_bus_scan(i2c_bus, found, sizeof(found));
        ESP_LOGI(TAG, "i2c devices found: %d", count);
        for (uint8_t i = 0; i < count; i++)
        {
            ESP_LOGI(TAG, "addr: 0x%02X", found[i]);
        }

        // Step2: Init bme280
        bme280_ = bme280_create(i2c_bus, BME280_I2C_ADDRESS_DEFAULT);
        bme280_default_init(bme280_);
        vTaskDelay(pdMS_TO_TICKS(200));

        // Step3: Read temperature, humidity and pressure
        if (xTaskCreate(updateTask, "Log Meteo", 2048, this, 3, NULL) != pdPASS)
        {
            abort();
        }
    }

    string_value_t MeteoHandler::getTemperatureString() const
    {
        return convertValueToString(tempX100_, Format::TwoDecimals, Unit::Degree);
    }

    string_value_t MeteoHandler::getHumidityString() const
    {
        return convertValueToString(humidX100_, Format::TwoDecimals, Unit::Percent);
    }

    string_value_t MeteoHandler::getPressureString() const
    {
        return convertValueToString(pressure_, Format::Integer, Unit::Mm);
    }

    void MeteoHandler::setCallbackOnTemperatureUpdate(EventCallback cb)
    {
        onTempUpdateCb_ = cb;
    };
    void MeteoHandler::setCallbackOnHumidityUpdate(EventCallback cb)
    {
        onHumidUpdateCb_ = cb;
    };
    void MeteoHandler::setCallbackOnPressureUpdate(EventCallback cb)
    {
        onPressUpdateCb_ = cb;
    };

    void MeteoHandler::updateTask(void *pvParams)
    {
        auto *self = static_cast<MeteoHandler *>(pvParams);
        self->updateLoop();
    }

    void MeteoHandler::updateTemperature()
    {
        float value = 0.0f;
        if (bme280_read_temperature(bme280_, &value) != ESP_OK)
        {
            ESP_LOGE(TAG, "No Temperature update!");
            return;
        }

        ESP_LOGI(TAG, "Temperature value: %f", value);
        int32_t roundedValue = lroundf(value * 100.0f);
        ESP_LOGI(TAG, "Temperature roundedValue: %d", roundedValue);
        if (roundedValue == tempX100_)
            return;

        tempX100_ = roundedValue;
        if (onTempUpdateCb_)
            onTempUpdateCb_(getTemperatureString().data());
    }

    void MeteoHandler::updateHumidity()
    {
        float value = 0.0f;
        if (bme280_read_humidity(bme280_, &value) != ESP_OK)
        {
            ESP_LOGE(TAG, "No Humidity update!");
            return;
        }

        ESP_LOGI(TAG, "Humidity value: %f", value);

        int32_t roundedValue = lroundf(value * 100.0f);
        ESP_LOGI(TAG, "Humidity roundedValue: %d", roundedValue);
        if (roundedValue == humidX100_)
            return;

        humidX100_ = roundedValue;
        if (onHumidUpdateCb_)
            onHumidUpdateCb_(getHumidityString().data());
    }

    void MeteoHandler::updatePressure()
    {
        float value = 0.0f;
        if (bme280_read_pressure(bme280_, &value) != ESP_OK)
            return;

        int32_t roundedValue = lroundf(value);
        if (roundedValue == pressure_)
            return;

        pressure_ = roundedValue;
        if (onPressUpdateCb_)
            onPressUpdateCb_(getPressureString().data());
    }

    void MeteoHandler::updateLoop()
    {
        while (true)
        {
            updateTemperature();
            vTaskDelay(pdMS_TO_TICKS(50));
            updateHumidity();
            vTaskDelay(pdMS_TO_TICKS(50));
            updatePressure();

            vTaskDelay(pdMS_TO_TICKS(meteoConfig::refreshIntervalMs));
        }
    }

    // temp: -99.99 .. 99.99 (6 chars max + \0)
    // humidity: 0 .. 100.00 (6 chars max + \0)
    // pressure: 0 .. 9999 (4 chars max + \0)
    string_value_t MeteoHandler::convertValueToString(int32_t value, Format format, Unit unit) const
    {
        value = std::clamp<int32_t>(value, -9999, 99999);
        string_value_t buf{};
        const char *unitStr = unitToString(unit);
        const char *sign = (value < 0) ? "-" : "";
        uint32_t abs_val = (value < 0) ? -static_cast<uint32_t>(value)
                                       : static_cast<uint32_t>(value);

        if (format == Format::Integer)
        {
            snprintf(buf.data(), buf.size(), "%s%lu%s", sign,
                     static_cast<unsigned long>(abs_val), unitStr);
        }
        else if (format == Format::TwoDecimals)
        {
            uint32_t int_part = abs_val / 100;
            uint32_t frac_part = abs_val % 100;

            snprintf(buf.data(), buf.size(), "%s%lu.%02lu%s", sign,
                     static_cast<unsigned long>(int_part),
                     static_cast<unsigned long>(frac_part), unitStr);
        }

        return buf;
    }
} // namespace meteo