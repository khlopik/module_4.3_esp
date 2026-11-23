#include "lcd.hpp"
#include "esp_lvgl_port.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "config.hpp"
#include "esp_lcd_panel_vendor.h"
#include <ssd1306.h>

namespace
{
    const char *TAG = "LCD";
} // namespace

namespace lcd
{
    void LcdHandler::init(i2c_master_bus_handle_t masterBus)
    {
        ESP_LOGI(TAG, "Install panel IO");

        esp_lcd_panel_io_handle_t io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t io_config = {};
        io_config.dev_addr = lcdConfig::i2cAddress;
        io_config.control_phase_bytes = 1;               // According to SSD1306 datasheet
        io_config.lcd_cmd_bits = lcdConfig::cmdBits;     // According to SSD1306 datasheet
        io_config.lcd_param_bits = lcdConfig::paramBits; // According to SSD1306 datasheet
        io_config.dc_bit_offset = 6;
        io_config.scl_speed_hz = i2cConfig::clockSpeedHz;

        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(masterBus, &io_config, &io_handle));

        ESP_LOGI(TAG, "Install SSD1306 panel driver");
        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.bits_per_pixel = 1;
        panel_config.reset_gpio_num = i2cConfig::rstPin;

        esp_lcd_panel_ssd1306_config_t ssd1306_config = {};
        ssd1306_config.height = lcdConfig::vRes;
        panel_config.vendor_config = &ssd1306_config;

        ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));

        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

        ESP_LOGI(TAG, "Initialize LVGL");

        const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
        lvgl_port_init(&lvgl_cfg);

        lvgl_port_display_cfg_t disp_cfg = {};
        disp_cfg.io_handle = io_handle;
        disp_cfg.panel_handle = panel_handle;
        disp_cfg.buffer_size = lcdConfig::hRes * lcdConfig::vRes;
        disp_cfg.double_buffer = true;
        disp_cfg.hres = lcdConfig::hRes;
        disp_cfg.vres = lcdConfig::vRes;
        disp_cfg.monochrome = true;
        disp_cfg.rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = true,
        };

        disp_ = lvgl_port_add_disp(&disp_cfg);
        lv_disp_set_rotation(disp_, LV_DISP_ROT_NONE);

        // initBar(redBar_);
        // initBar(greenBar_);
        // initBar(blueBar_);

        initMeteoItem(temperature_);
        initMeteoItem(humidity_);
        initMeteoItem(pressure_);
    }

    void LcdHandler::initBar(ColorBar &bar)
    {
        ESP_LOGI(TAG, "initBar");
        if (disp_ && lvgl_port_lock(0))
        {
            lv_obj_t *scr = lv_disp_get_scr_act(disp_);

            bar.rect = lv_obj_create(scr);
            lv_obj_set_size(bar.rect, bar.width, lcdConfig::initialBarHeight);
            lv_obj_set_style_bg_color(bar.rect, lv_color_black(), 0);
            lv_obj_set_style_bg_opa(bar.rect, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(bar.rect, 0, 0);
            lv_obj_set_pos(bar.rect, bar.posX, bar.maxHeight - lcdConfig::initialBarHeight);

            lv_obj_t *label = lv_label_create(scr);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
            lv_label_set_text(label, bar.label);
            lv_obj_set_pos(label, bar.posX - 20, bar.posY + bar.maxHeight - 20);

            bar.valueLabel = lv_label_create(scr);
            lv_obj_set_style_text_font(bar.valueLabel, &lv_font_montserrat_10, 0);
            lv_label_set_text(bar.valueLabel, "127");
            lv_obj_set_pos(bar.valueLabel, bar.posX - 20, bar.posY + bar.maxHeight - 20 - 15);

            lvgl_port_unlock();
        }
    }

    void LcdHandler::initMeteoItem(MeteoItem &meteoItem)
    {
        ESP_LOGI(TAG, "initMeteoItem");
        if (disp_ && lvgl_port_lock(0))
        {
            lv_obj_t *scr = lv_disp_get_scr_act(disp_);

            lv_obj_t *label = lv_label_create(scr);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
            lv_label_set_text(label, meteoItem.label);
            lv_obj_set_pos(label, meteoItem.posX, meteoItem.posY);

            meteoItem.valueLabel = lv_label_create(scr);
            lv_obj_set_style_text_font(meteoItem.valueLabel, &lv_font_montserrat_20, 0);
            lv_label_set_text(meteoItem.valueLabel, "-");
            lv_obj_set_pos(meteoItem.valueLabel, meteoItem.posX + 40, meteoItem.posY);

            lvgl_port_unlock();
        }
    }

    void LcdHandler::updateBarData(Color color, uint8_t height, uint8_t value)
    {
        ColorBar *bar = nullptr;
        switch (color)
        {
        case Color::Red:
            bar = &redBar_;
            break;
        case Color::Green:
            bar = &greenBar_;
            break;
        case Color::Blue:
            bar = &blueBar_;
            break;
        default:
            break;
        }
        if (bar && bar->rect && lvgl_port_lock(0))
        {
            lv_obj_set_size(bar->rect, bar->width, height);
            lv_obj_set_pos(bar->rect, bar->posX, bar->maxHeight - height);

            char buf[4];
            snprintf(buf, sizeof(buf), "%u", value);
            lv_label_set_text(bar->valueLabel, buf);

            lvgl_port_unlock();
        }
    }

    void LcdHandler::updateMeteoItem(MeteoUnit unit, const char *stringValue)
    {
        MeteoItem *item = nullptr;
        switch (unit)
        {
        case MeteoUnit::Temperature:
            item = &temperature_;
            break;
        case MeteoUnit::Humidity:
            item = &humidity_;
            break;
        case MeteoUnit::Pressure:
            item = &pressure_;
            break;
        default:
            break;
        }
        if (item && item->valueLabel && lvgl_port_lock(0))
        {
            lv_label_set_text(item->valueLabel, stringValue);

            lvgl_port_unlock();
        }
    }
} // namespace lcd