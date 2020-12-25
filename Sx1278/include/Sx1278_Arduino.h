#pragma once

#ifdef ARDUINO
#include "Sx1278.h"

class Sx1278_Arduino : public Sx1278
{
public:
  Sx1278_Arduino(uint8_t CsPin = 15, uint8_t Dio0Pin = 4, uint8_t ResetPin = 0) : Sx1278(CsPin, Dio0Pin, ResetPin) { }
  virtual void Init() override;

protected:
  virtual void Write(uint8_t Address, const uint8_t *Data, size_t DataLength);
  virtual void Read(uint8_t Address, size_t DataLength, uint8_t *OutBuffer);

  virtual void Wait(uint32_t Millis) override;
  virtual void SetupDIO0Interrupt() override;
  virtual void OnDIO0Interrupt() override;
  virtual void Log(const char *Text) override;
  virtual void DigitalWrite(uint8_t IO, uint8_t Value) override;
  virtual void PinMode(uint8_t IO, EIOMode Mode) override;
};
#endif