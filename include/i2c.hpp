#pragma once

#include "driver/i2c_master.h"

namespace i2c
{
    class I2CHandler
    {
    public:
        void init();
        i2c_master_bus_handle_t getMasterBus() const;
        void scan();

    private:
        i2c_master_bus_handle_t masterBus_;
    };
} // namespace i2c