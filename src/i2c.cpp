#include "i2c.hpp"
#include "driver/i2c_master.h"
#include "config.hpp"
#include "esp_log.h"

namespace
{
    const char *TAG = "I2C";
} // namespace

namespace i2c
{
    void I2CHandler::init()
    {
        ESP_LOGI(TAG, "Initialize I2C bus");

        i2c_master_bus_config_t bus_cfg = {};
        bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_cfg.i2c_port = i2cConfig::hostPort;
        bus_cfg.sda_io_num = i2cConfig::sdaPin;
        bus_cfg.scl_io_num = i2cConfig::sclPin;
        bus_cfg.glitch_ignore_cnt = 7;
        bus_cfg.flags.enable_internal_pullup = true;

        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &masterBus_));
    }

    i2c_master_bus_handle_t I2CHandler::getMasterBus() const
    {
        return masterBus_;
    }
}