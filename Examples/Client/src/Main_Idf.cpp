#ifdef ESP32_IDF

#include "esp_log.h"
#include "Sx1278_Idf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void MainTaskWithBigStackSize(void *)
{
    ESP_LOGI(LOG_TAG, "Client IDF");
    Sx1278_Idf RFM;

    RFM.Init();

    while (true)
    {
        static uint32_t MessageId = 0;
        if (RFM.CanTransmit())
        {
            RFM.TransmitData((const uint8_t *)&MessageId, sizeof(uint32_t));
            MessageId++;
        }

        RFM.Wait(1000);
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