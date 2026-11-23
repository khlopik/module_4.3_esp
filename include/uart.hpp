#pragma once

#include <cstdint>
#include <stddef.h>
#include <optional>
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"

namespace uart
{
    enum class TxCommand
    {
        ButtonPress,
        ButtonRelease,
        Unknown,
    };

    enum class CommandType
    {
        ON,
        OFF,
        R,
        G,
        B,
        Unknown
    };

    struct RxCommand
    {
        CommandType type;
        uint8_t value;
    };

    using CommandCallback = void (*)(RxCommand);

    class UartHandler
    {
    public:
        void init(gpio_num_t rxPin, gpio_num_t txPin);
        static void checkMessagesTask(void *pvParams);
        static void sendMessagesTask(void *pvParams);
        void setCallback(CommandCallback cb);
        void sendCommandToQueue(TxCommand cmd);

    private:
        void checkMessagesLoop();
        void sendMessagesLoop();
        RxCommand parseCommand(const char *str);

        static constexpr int bufferSize_ = (1024 * 2);
        QueueHandle_t queue_;
        static constexpr uart_port_t portNum_ = UART_NUM_2;

        static constexpr size_t MAX_MSG_LEN = 32;
        char buffer_[MAX_MSG_LEN]{};
        size_t bufIndex_ = 0;

        QueueHandle_t messageQueue_ = nullptr;
        static constexpr UBaseType_t QUEUE_LENGTH = 4;

        CommandCallback callback_ = nullptr;
    }; // class UartHandler
} // namespace uart