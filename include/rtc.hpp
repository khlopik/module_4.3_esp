#pragma once

#include "driver/i2c_master.h"
#include <time.h>

namespace rtc
{
    uint8_t valueToBcd(uint8_t value);

    struct DateTime
    {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t day;
        uint8_t month;
        uint8_t year;
    };

    class RTCHandler
    {
    public:
        void init(i2c_master_bus_handle_t masterBus);
        void resetAndRun();
        void updateSeconds();
        void update();
        void setTime(uint8_t *data);
        void printTime();

    private:
        i2c_master_dev_handle_t devHandle_;
        uint8_t secondsAddr_ = 0x00;
        uint8_t seconds_;
        DateTime time_;
        time_t systemTime_;
    };
}