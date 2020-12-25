#ifdef ESP32_IDF

#include "esp_log.h"
#include "Sx1278_Idf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void MainTaskWithBigStackSize(void*)
{
    ESP_LOGI(LOG_TAG, "Client IDF");
    Sx1278_Idf RFM;

    RFM.Init();

    while (true);
}

extern "C" void app_main()
{
    TaskHandle_t TaskHandle;
     xTaskCreate(
        &MainTaskWithBigStackSize,        /* Function that implements the task. */
        "IdfClient",           /* Text name for the task. */
        4096 * 2,       /* Stack size in words, not bytes. */
        nullptr,        /* Parameter passed into the task. */
        tskIDLE_PRIORITY, /* Priority at which the task is created. */
        &TaskHandle);        /* Used to pass out the created task's handle. */

}

#endif