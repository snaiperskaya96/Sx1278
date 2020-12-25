#pragma once

#ifdef ESP32_IDF
#include "Sx1278.h"

#include <mutex>

#define LOG_TAG "[Sx1278_Idf]"

typedef struct spi_device_t* spi_device_handle_t;  ///< Handle for a device on a SPI bus
typedef void* QueueHandle_t;
typedef QueueHandle_t SemaphoreHandle_t;

class Sx1278_Idf : public Sx1278
{
public:
  Sx1278_Idf(uint8_t Miso = 19, uint8_t Mosi = 23, uint8_t Sck = 18, uint8_t CsPin = 5, uint8_t Dio0Pin = 14, uint8_t ResetPin = 0) : Sx1278(CsPin, Dio0Pin, ResetPin), Miso(Miso), Mosi(Mosi), Sck(Sck) {}
  virtual void Init() override;
  void OnDIO0InterruptPub();

  virtual void DigitalWrite(uint8_t IO, uint8_t Value) override;
  virtual void PinMode(uint8_t IO, EIOMode Mode) override;
  virtual void Wait(uint32_t Millis) override;

  void CreateInterruptSemaphore();
  SemaphoreHandle_t GetInterruptSemaphore() const { return InterruptSemaphore; }
protected:
  virtual void Write(uint8_t Address, const uint8_t *Data, size_t DataLength);
  virtual void Read(uint8_t Address, size_t DataLength, uint8_t *OutBuffer);

  virtual void SetupDIO0Interrupt() override;
  virtual void OnDIO0Interrupt() override;
  virtual void Log(const char *Text) override;

  uint8_t Transfer(uint8_t data);
protected:
  uint8_t Miso;
  uint8_t Mosi;
  uint8_t Sck;
  static std::mutex SpiMutex;
  spi_device_handle_t SpiHandle = nullptr;
  SemaphoreHandle_t InterruptSemaphore = nullptr;
};
#endif