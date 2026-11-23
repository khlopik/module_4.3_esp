#include "config.hpp"
#include "driver/uart.h"
#include "button.hpp"
#include "led.hpp"
#include "uart.hpp"
#include "esp_log.h"
#include "config.hpp"
#include "i2c.hpp"
#include "lcd.hpp"
#include "meteo.hpp"
#include "rtc.hpp"
#include "freertos/FreeRTOS.h"

namespace
{
    const char *TAG = "MAIN";

    button::ButtonHandler buttonHandler;
    led::LedHandler ledHandle;
    lcd::LcdHandler lcdHandle;
    uart::UartHandler uartHandler;
    meteo::MeteoHandler meteoHandler;
    i2c::I2CHandler i2cHandler;
    rtc::RTCHandler rtcHandler;

    uint8_t convertRGBValueToBarHeight(uint8_t value)
    {
        return value * lcdConfig::maxBarHeight / ledConfig::maxColorValue;
    }

    void onSTMCommand(uart::RxCommand cmd)
    {
        ESP_LOGI(TAG, "onSTMCommand");
        lcd::Color colorToUpdate = lcd::Color::Unknown;

        switch (cmd.type)
        {
        case uart::CommandType::OFF:
            ledHandle.turnOff();
            break;
        case uart::CommandType::ON:
            ledHandle.turnOn();
            break;
        case uart::CommandType::R:
            ledHandle.setColor(led::Color::Red, cmd.value);
            colorToUpdate = lcd::Color::Red;
            break;
        case uart::CommandType::G:
            ledHandle.setColor(led::Color::Green, cmd.value);
            colorToUpdate = lcd::Color::Green;
            break;
        case uart::CommandType::B:
            ledHandle.setColor(led::Color::Blue, cmd.value);
            colorToUpdate = lcd::Color::Blue;
            break;
        default:
            break;
        }

        if (colorToUpdate != lcd::Color::Unknown)
        {
            lcdHandle.updateBarData(colorToUpdate, convertRGBValueToBarHeight(cmd.value), cmd.value);
        }
    }

    void onButtonPress(void *arg, void *data)
    {
        uartHandler.sendCommandToQueue(uart::TxCommand::ButtonPress);
    }

    void OnButtonRelease(void *arg, void *data)
    {
        uartHandler.sendCommandToQueue(uart::TxCommand::ButtonRelease);
    }

    void onTemperatureUpdate(const char *stringValue)
    {
        lcdHandle.updateMeteoItem(lcd::MeteoUnit::Temperature, stringValue);
    }

    void onHumidityUpdate(const char *stringValue)
    {
        lcdHandle.updateMeteoItem(lcd::MeteoUnit::Humidity, stringValue);
    }

    void onPressureUpdate(const char *stringValue)
    {
        lcdHandle.updateMeteoItem(lcd::MeteoUnit::Pressure, stringValue);
    }
}

void setup()
{
    ledHandle.init();
    i2cHandler.init();
    buttonHandler.init(buttonConfig::pin, buttonConfig::shortPressTimeMs, buttonConfig::activeLevel);
    buttonHandler.setOnPressCallback(&onButtonPress);
    buttonHandler.setOnReleaseCallback(&OnButtonRelease);
    uartHandler.init(uartConfig::rxPin, uartConfig::txPin);
    vTaskDelay(pdMS_TO_TICKS(1000));
    uartHandler.setCallback(&onSTMCommand);
    i2cHandler.scan();
    lcdHandle.init(i2cHandler.getMasterBus());
    meteoHandler.init();
    meteoHandler.setCallbackOnTemperatureUpdate(&onTemperatureUpdate);
    meteoHandler.setCallbackOnHumidityUpdate(&onHumidityUpdate);
    meteoHandler.setCallbackOnPressureUpdate(&onPressureUpdate);
    rtcHandler.init(i2cHandler.getMasterBus());

    // rtcHandler.resetAndRun();
    uint8_t timeData[8] = {
        0x00,
        rtc::valueToBcd(0),
        rtc::valueToBcd(45),
        rtc::valueToBcd(15),
        rtc::valueToBcd(1),
        rtc::valueToBcd(23),
        rtc::valueToBcd(11),
        rtc::valueToBcd(26),
    };
    // rtcHandler.setTime(timeData);
    vTaskDelay(pdMS_TO_TICKS(3000));
    rtcHandler.update();
    // rtcHandler.updateSeconds();
    vTaskDelay(pdMS_TO_TICKS(3000));
    rtcHandler.printTime();
    vTaskDelay(pdMS_TO_TICKS(3000));
    rtcHandler.printTime();
    vTaskDelay(pdMS_TO_TICKS(3000));
    rtcHandler.printTime();
    // rtcHandler.updateSeconds();
    // vTaskDelay(pdMS_TO_TICKS(3000));
    // rtcHandler.updateSeconds();
}

void app()
{
}

extern "C" void app_main()
{
    setup();
}