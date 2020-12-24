#include "RFM95Arduino.h"

#include <Arduino.h>
#include <SPI.h>
#include <FunctionalInterrupt.h>

// MSB first!
SPISettings Settings = SPISettings(1000000, MSBFIRST, SPI_MODE0);

void RFM95Arduino::DigitalWrite(uint8_t IO, uint8_t Value)
{
    digitalWrite(IO, Value);
}

void RFM95Arduino::Write(uint8_t Address, const uint8_t *Data, size_t DataLength)
{
    SPI.beginTransaction(Settings);
    DigitalWrite(CsPin, 0);
    SPI.transfer(Address | SPI_WRITE);
    for (size_t I = 0; I < DataLength; I++)
    {
        SPI.transfer(Data[I]);
    }
    DigitalWrite(CsPin, 1);
    SPI.endTransaction();
}

void RFM95Arduino::Read(uint8_t Address, size_t DataLength, uint8_t *OutBuffer)
{
    SPI.beginTransaction(Settings);
    DigitalWrite(CsPin, 0);
    SPI.transfer(Address & ~SPI_WRITE);
    for (size_t I = 0; I < DataLength; I++)
    {
        OutBuffer[I] = SPI.transfer(0);
    }
    DigitalWrite(CsPin, 1);
    SPI.endTransaction();
}

void RFM95Arduino::Wait(uint32_t Millis)
{
    delay(Millis);
}

void RFM95Arduino::SetupDIO0Interrupt()
{
    attachInterrupt(digitalPinToInterrupt(Dio0Pin), std::bind(&RFM95Arduino::OnDIO0Interrupt, this), RISING);
}

void RFM95Arduino::OnDIO0Interrupt()
{
    RFM95::OnDIO0Interrupt();
}

void RFM95Arduino::Log(const char *Text)
{
    if (Serial)
    {
        Serial.print("[RFM95Arduino] ");
        Serial.println(Text);
    }
}

void RFM95Arduino::PinMode(uint8_t IO, EIOMode Mode)
{
    switch (Mode)
    {
    case EIOMode::Input:
        pinMode(IO, INPUT);
        break;
    case EIOMode::Output:
        pinMode(IO, OUTPUT);
        break;
    case EIOMode::PullUp:
        pinMode(IO, INPUT_PULLUP);
        break;
    default:
        break;
    }
}

void RFM95Arduino::Init()
{
    SPI.begin();

    RFM95::Init();
}