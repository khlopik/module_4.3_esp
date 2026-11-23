#include "rtc.hpp"
#include "config.hpp"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>

namespace
{
    const char *TAG = "RTC";

    uint8_t bcdToValue(uint8_t bcd)
    {
        return ((bcd >> 4) * 10) + (bcd & 0xF);
    }
} // namespace

namespace rtc
{
    uint8_t valueToBcd(uint8_t value)
    {
        return ((value / 10) << 4) | (value % 10);
    }

    void RTCHandler::init(i2c_master_bus_handle_t masterBus)
    {
        i2c_device_config_t dev_config = {};
        dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_config.device_address = 0x68;
        dev_config.scl_speed_hz = i2cConfig::clockSpeedHz;

        ESP_ERROR_CHECK(i2c_master_bus_add_device(masterBus, &dev_config, &devHandle_));

        if (!devHandle_)
        {
            ESP_LOGE(TAG, "RTC module has not been initialized");
        }
        else
        {
            ESP_LOGI(TAG, "RTC module has been successfully initialized");
        }
    }

    void RTCHandler::resetAndRun()
    {
        uint8_t data[2] = {secondsAddr_, 0x00};
        i2c_master_transmit(devHandle_, data, 2, -1);
    }

    void RTCHandler::updateSeconds()
    {
        esp_err_t ret = i2c_master_transmit_receive(devHandle_, &secondsAddr_, 1, &seconds_, 1, -1);
        if (ret == ESP_OK)
        {
            printf("Seconds: %d\n", seconds_);
        }
        // 3. Транзакції
        // uint8_t write_buf[] = {0x00, 0xAF};
        // uint8_t read_buf[4] = {0};

        // Запис
        // ESP_ERROR_CHECK(i2c_master_transmit(devHandle_, write_buf, sizeof(write_buf), -1));

        // Читання
        // ESP_ERROR_CHECK(i2c_master_receive(devHandle_, read_buf, sizeof(read_buf), -1));

        // Запис з наступним читанням без STOP-біта (Repeated Start — для читання регістрів)
        // uint8_t reg_addr = 0xD0;
        // ESP_ERROR_CHECK(i2c_master_transmit_receive(devHandle_, &reg_addr, 1, read_buf, sizeof(read_buf), -1));
    }

    void RTCHandler::update()
    {
        uint8_t read_buf[7] = {};
        esp_err_t ret = i2c_master_transmit_receive(devHandle_, &secondsAddr_, 1, read_buf, sizeof(read_buf), -1);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Error while trying to update time: %d", ret);
            return;
        }
        time_.seconds = bcdToValue(read_buf[0]);
        time_.minutes = bcdToValue(read_buf[1]);
        time_.hours = bcdToValue(read_buf[2]);
        time_.day = bcdToValue(read_buf[4]);
        time_.month = bcdToValue(read_buf[5]);
        time_.year = bcdToValue(read_buf[6]);
        ESP_LOGI(TAG, "DateTime: %d-%d-%d %s%d:%s%d:%s%d", time_.year, time_.month, time_.day, time_.hours < 10 ? "0" : "", time_.hours, time_.minutes < 10 ? "0" : "", time_.minutes, time_.seconds < 10 ? "0" : "", time_.seconds);
        tm timeInfo = {};
        timeInfo.tm_sec = time_.seconds;
        timeInfo.tm_min = time_.minutes;
        timeInfo.tm_hour = time_.hours;
        timeInfo.tm_mday = time_.day;
        timeInfo.tm_mon = time_.month - 1;
        timeInfo.tm_year = 2000 + time_.year;
        systemTime_ = mktime(&timeInfo);
        const timeval systemTime = {
            .tv_sec = systemTime_,
            .tv_usec = 0,
        };
        settimeofday(&systemTime, nullptr);
    }

    void RTCHandler::setTime(uint8_t *data)
    {
        ESP_ERROR_CHECK(i2c_master_transmit(devHandle_, data, 8, -1));
    }

    void RTCHandler::printTime()
    {
        time_t currentTime;
        time(&currentTime);
        tm timeInfo = {};
        localtime_r(&currentTime, &timeInfo);
        ESP_LOGI(TAG, "DateTime: %d-%d-%d %s%d:%s%d:%s%d", timeInfo.tm_year, timeInfo.tm_mon + 1, timeInfo.tm_mday, timeInfo.tm_hour < 10 ? "0" : "", timeInfo.tm_hour, timeInfo.tm_min < 10 ? "0" : "", timeInfo.tm_min, timeInfo.tm_sec < 10 ? "0" : "", timeInfo.tm_sec);
    }
}