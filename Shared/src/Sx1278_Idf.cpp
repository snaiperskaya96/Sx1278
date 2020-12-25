#define ESP32_IDF 1
#ifdef ESP32_IDF
#include "Sx1278_Idf.h"
#include "Util.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "soc/spi_struct.h"

#include <cstring>

// MSB first!
//SPISettings Settings = SPISettings(1000000, MSBFIRST, SPI_MODE0);

std::mutex Sx1278_Idf::SpiMutex;

void Sx1278_Idf::DigitalWrite(uint8_t IO, uint8_t Value)
{
    gpio_set_level((gpio_num_t)IO, Value);
}

// Not the most performant but at least on the same level of the Arduino implementation
uint8_t Sx1278_Idf::Transfer(uint8_t Data)
{
    if (!SpiHandle)
    {
        return 0;
    }

    uint8_t Out = 0;

    SpiMutex.lock();
    spi_transaction_t t = {};
    t.length = 8;        //Command is 8 bits
    t.tx_buffer = &Data; //The data is the cmd itself
    t.rx_buffer = &Out;
    t.user = nullptr;
    ESP_ERROR_CHECK(spi_device_polling_transmit(SpiHandle, &t)); //Transmit!
    SpiMutex.unlock();

    return Out;
}

void Sx1278_Idf::Write(uint8_t Address, const uint8_t *Data, size_t DataLength)
{
    SpiMutex.lock();
    DigitalWrite(CsPin, 0);

    spi_transaction_t transaction = {};
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = Address | SPI_WRITE;
    transaction.length = DataLength * 8;
    transaction.rxlength = 0;
    transaction.user = nullptr;
    transaction.tx_buffer = Data;
    transaction.rx_buffer = NULL;
    spi_device_transmit(SpiHandle, &transaction);

    DigitalWrite(CsPin, 1);
    SpiMutex.unlock();
}

void Sx1278_Idf::Read(uint8_t Address, size_t DataLength, uint8_t *OutBuffer)
{
    SpiMutex.lock();
    DigitalWrite(CsPin, 0);

    spi_transaction_t transaction = {};
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = Address & ~SPI_WRITE;
    transaction.length = DataLength * 8;
    transaction.rxlength = DataLength * 8;
    transaction.user = nullptr;
    transaction.tx_buffer = nullptr;
    transaction.rx_buffer = OutBuffer;
    spi_device_transmit(SpiHandle, &transaction);

    DigitalWrite(CsPin, 1);
    SpiMutex.unlock();
}

void Sx1278_Idf::Wait(uint32_t Millis)
{
    const TickType_t Delay = Millis / portTICK_PERIOD_MS;
    vTaskDelay(Delay);
}

// ISR 
void OnInterrupt(void *Args)
{
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (Sx1278_Idf *Sx = static_cast<Sx1278_Idf *>(Args))
    {
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(Sx->GetInterruptSemaphore(), &xHigherPriorityTaskWoken);
    }
}

void Sx1278_Idf::OnDIO0InterruptPub()
{
    OnDIO0Interrupt();
}

void Sx1278_Idf::SetupDIO0Interrupt()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ULL << Dio0Pin;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add((gpio_num_t)Dio0Pin, (gpio_isr_t)&OnInterrupt, this);
}

void Sx1278_Idf::OnDIO0Interrupt()
{
    Sx1278::OnDIO0Interrupt();
}

void Sx1278_Idf::Log(const char *Text)
{
    ESP_LOGI("[Sx1278_Idf]", "%s", Text);
}

void Sx1278_Idf::PinMode(uint8_t IO, EIOMode Mode)
{
    switch (Mode)
    {
    case EIOMode::Input:
        gpio_set_direction((gpio_num_t)IO, GPIO_MODE_INPUT);
        break;
    case EIOMode::Output:
        gpio_set_direction((gpio_num_t)IO, GPIO_MODE_OUTPUT);
        break;
    case EIOMode::PullUp:
        gpio_set_pull_mode((gpio_num_t)IO, gpio_pull_mode_t::GPIO_PULLUP_ONLY);
        break;
    default:
        break;
    }
}

void Sx1278_Idf::CreateInterruptSemaphore()
{
    InterruptSemaphore = xSemaphoreCreateBinary();
}

void InterruptTask(void *This)
{
    if (Sx1278_Idf *Idf = static_cast<Sx1278_Idf *>(This))
    {
        Idf->CreateInterruptSemaphore();
        while (true)
        {
            if (xSemaphoreTake(Idf->GetInterruptSemaphore(), portMAX_DELAY) == pdTRUE)
            {
                Idf->OnDIO0InterruptPub();
            }
        }
    }
}

void Sx1278_Idf::Init()
{
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = Miso;
    buscfg.mosi_io_num = Mosi;
    buscfg.sclk_io_num = Sck;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 0));

    spi_device_interface_config_t dev_config = {};
    dev_config.command_bits = 0;
    dev_config.address_bits = 8;
    dev_config.dummy_bits = 0;
    dev_config.mode = 0;
    dev_config.duty_cycle_pos = 128; // default 128 = 50%/50% duty
    dev_config.cs_ena_pretrans = 0;  // 0 not used
    dev_config.cs_ena_posttrans = 0; // 0 not used
    dev_config.clock_speed_hz = 1000000;
    dev_config.spics_io_num = CsPin;
    dev_config.flags = 0; // 0 not used
    dev_config.queue_size = 1;
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;

    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &SpiHandle));

    TaskHandle_t TaskHandle;
    xTaskCreate(
        &InterruptTask,     /* Function that implements the task. */
        "IdfInterruptTask", /* Text name for the task. */
        2048,                /* Stack size in words, not bytes. */
        this,               /* Parameter passed into the task. */
        tskIDLE_PRIORITY,   /* Priority at which the task is created. */
        &TaskHandle);       /* Used to pass out the created task's handle. */

    Sx1278::Init();
}
#endif