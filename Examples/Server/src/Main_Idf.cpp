#ifdef ESP32_IDF

#include "esp_log.h"
#include "Sx1278_Idf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstring>

bool bLedOn = false;
uint8_t LedPin = 22;
uint32_t LedTime = 0;

void MainTaskWithBigStackSize(void *)
{
    ESP_LOGI(LOG_TAG, "Client IDF");
    Sx1278_Idf RFM;

    RFM.Init();
    RFM.Receive();
    RFM.PinMode(LedPin, EIOMode::Output);

    while (true)
    {
        if (RFM.IsMessageAvailable())
        {
            if (IncomingMessage *Message = RFM.GetNextIncomingMessage())
            {
                uint32_t MessageId = 0;
                memcpy(&MessageId, Message->Buffer, sizeof(uint32_t));
                ESP_LOGI("", "Message received. Id: %d", MessageId);
                bLedOn = true;
            }
        }

        RFM.DigitalWrite(LedPin, bLedOn ? 1 : 0);
        RFM.Wait(1);
        if (bLedOn)
        {
            LedTime += 1;
            if (LedTime >= 200)
            {
                LedTime = 0;
                bLedOn = false;
            }
        }
    }
}

extern "C" void app_main()
{
    TaskHandle_t TaskHandle;
    xTaskCreate(
        &MainTaskWithBigStackSize, /* Function that implements the task. */
        "IdfClient",               /* Text name for the task. */
        4096 * 2,                  /* Stack size in words, not bytes. */
        nullptr,                   /* Parameter passed into the task. */
        tskIDLE_PRIORITY,          /* Priority at which the task is created. */
        &TaskHandle);              /* Used to pass out the created task's handle. */
}

#endif