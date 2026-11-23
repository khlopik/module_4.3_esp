#include "uart.hpp"
#include "esp_log.h"

namespace
{
    const char *TAG = "UART";
} // namespace

namespace uart
{
    void UartHandler::init(gpio_num_t rxPin, gpio_num_t txPin)
    {
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, bufferSize_, bufferSize_, 10, &queue_, 0));

        uart_config_t uart_config = {};
        uart_config.baud_rate = 115200;
        uart_config.data_bits = UART_DATA_8_BITS;
        uart_config.parity = UART_PARITY_DISABLE;
        uart_config.stop_bits = UART_STOP_BITS_1;
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_config.rx_flow_ctrl_thresh = 122;

        ESP_ERROR_CHECK(uart_param_config(portNum_, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

        messageQueue_ = xQueueCreate(QUEUE_LENGTH, sizeof(RxCommand));

        if (xTaskCreate(checkMessagesTask, "Check messages", 2048, this, 2, NULL) != pdPASS)
        {
            abort();
        }

        if (xTaskCreate(sendMessagesTask, "Send messages", 2048, this, 3, NULL) != pdPASS)
        {
            abort();
        }
    }

    void UartHandler::checkMessagesTask(void *pvParams)
    {
        auto *self = static_cast<UartHandler *>(pvParams);
        self->checkMessagesLoop();
    }

    void UartHandler::checkMessagesLoop()
    {
        uint8_t rxChunk[MAX_MSG_LEN];
        while (true)
        {
            int len = uart_read_bytes(UART_NUM_2, rxChunk, sizeof(rxChunk), pdMS_TO_TICKS(100));

            for (int i = 0; i < len; ++i)
            {
                char c = static_cast<char>(rxChunk[i]);

                if (c == '\r')
                {
                    continue;
                }

                if (c == '\n')
                {
                    buffer_[bufIndex_] = '\0';

                    if (bufIndex_ > 0)
                    {
                        RxCommand cmd = parseCommand(buffer_);
                        if (cmd.type != CommandType::Unknown && callback_ != nullptr)
                        {
                            callback_(cmd);
                        }
                    }

                    bufIndex_ = 0;
                }
                else if (bufIndex_ < MAX_MSG_LEN - 1)
                {
                    buffer_[bufIndex_++] = c;
                }
                else
                {
                    bufIndex_ = 0;
                }
            }
        }
    }

    void UartHandler::sendMessagesTask(void *pvParams)
    {
        auto *self = static_cast<UartHandler *>(pvParams);
        self->sendMessagesLoop();
    }

    void UartHandler::sendMessagesLoop()
    {
        TxCommand command;

        while (true)
        {
            if (xQueueReceive(messageQueue_, &command, portMAX_DELAY) == pdPASS)
            {
                ESP_LOGI(TAG, "sendMessage()");

                if (command == TxCommand::Unknown)
                {
                    continue;
                }

                const char *stringCommand;
                switch (command)
                {
                case TxCommand::ButtonPress:
                    stringCommand = "PRESS";
                    break;
                case TxCommand::ButtonRelease:
                    stringCommand = "RELEASE";
                    break;
                case TxCommand::Unknown:
                default:
                    stringCommand = "Unknown";
                }

                ESP_LOGI(TAG, "stringCommand: %s", stringCommand);
                char tx[16];
                int len = snprintf(tx, sizeof(tx), "%s\n", stringCommand);
                uart_write_bytes(portNum_, tx, len);
            }
        }
    }

    void UartHandler::setCallback(CommandCallback cb)
    {
        callback_ = cb;
    }

    void UartHandler::sendCommandToQueue(TxCommand cmd)
    {
        xQueueSend(messageQueue_, &cmd, portMAX_DELAY);
    }

    RxCommand UartHandler::parseCommand(const char *str)
    {
        RxCommand result{CommandType::Unknown, 0};

        // 1. Шукаємо роздільник ':'
        const char *colon = strchr(str, ':');
        if (colon == nullptr)
        {
            return result;
        }

        // 2. Визначаємо довжину частини з типом (до ':')
        size_t typeLen = static_cast<size_t>(colon - str);
        if (typeLen == 0)
        {
            return result;
        }

        // 3. Визначаємо Type за префіксом
        if (typeLen == 1 && str[0] == 'R')
        {
            result.type = CommandType::R;
        }
        else if (typeLen == 1 && str[0] == 'G')
        {
            result.type = CommandType::G;
        }
        else if (typeLen == 1 && str[0] == 'B')
        {
            result.type = CommandType::B;
        }
        else if (typeLen == 2 && strncmp(str, "ON", 2) == 0)
        {
            result.type = CommandType::ON;
        }
        else if (typeLen == 3 && strncmp(str, "OFF", 3) == 0)
        {
            result.type = CommandType::OFF;
        }
        else
        {
            return result;
        }

        // 4. Парсимо числове значення після ':'
        const char *valStr = colon + 1;
        if (*valStr == '\0')
        {
            result.type = CommandType::Unknown;
            return result;
        }

        char *endPtr = nullptr;
        long val = strtol(valStr, &endPtr, 10);

        // Перевіряємо, що весь залишок рядка був числом і входить у межі uint8_t (0..255)
        if (*endPtr != '\0' || val < 0 || val > 255)
        {
            result.type = CommandType::Unknown;
            return result;
        }

        result.value = static_cast<uint8_t>(val);
        return result;
    }
} // namespace uart