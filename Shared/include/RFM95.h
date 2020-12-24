#pragma once

#include <cstdint>
#include <cstdlib>

#ifndef MAX_INC_MESSAGES
#define MAX_INC_MESSAGES 10
#endif

enum class EOpMode : uint8_t
{
    Sleep = 0x0,
    Stdby = 0x1,
    FSTx = 0x2,
    Tx = 0x3,
    FSRx = 0x4,
    RxContinuous = 0x5,
    RxSingle = 0x6,
    ChannelActivityDetection = 0x7
};

enum class EIrqFlags : uint8_t
{
    CadDetected = 0x1,
    FhssChangeChannel = 0x2,
    CadDone = 0x4,
    TxDone = 0x8,
    ValidHeader = 0x10,
    PayloadCrcError = 0x20,
    RxDone = 0x40,
    RxTimeout = 0x80
};

enum class EIOMode
{
    Output,
    Input,
    PullUp
};

struct IncomingMessage
{
    uint8_t Buffer[256] = {0x0};
    uint8_t Size = 0;
    bool bRead = true;
};

class RFM95
{
public:
    RFM95(uint8_t CsPin = 15, uint8_t Dio0Pin = 4, uint8_t ResetPin = 0) : CsPin(CsPin), Dio0Pin(Dio0Pin), ResetPin(ResetPin) {}
    virtual void Init();
    virtual void Reset();
    bool IsMessageAvailable() const { return bMessageAvailable; }
    IncomingMessage* GetNextIncomingMessage();
    uint8_t GetNumMessageBuffers() const { return MAX_INC_MESSAGES; }
    void TransmitData(const uint8_t* Buffer, size_t BufferLength);
    // Timeout in milliseconds - 0 is infinite
    void Receive(uint32_t Timeout = 0);
    void SetMode(EOpMode Mode);
    void SetFrequency(float Frequency);
    void SetPreamble(uint16_t Preamble);
    bool CanTransmit();
    EOpMode GetMode();

    virtual uint8_t ReadRegister(uint8_t Address);
    virtual void WriteRegister(uint8_t Address, uint8_t Value);
protected:
    virtual void Write(uint8_t Address, const uint8_t* Data, size_t DataLength) = 0;
    virtual void Read(uint8_t Address, size_t DataLength, uint8_t* OutBuffer) = 0;

    virtual void Wait(uint32_t Millis) = 0;
    virtual void SetupDIO0Interrupt() = 0;
    virtual void DigitalWrite(uint8_t IO, uint8_t Value) = 0;
    virtual void PinMode(uint8_t IO, EIOMode Mode) = 0;

    virtual void OnDIO0Interrupt();
    virtual void Log(const char* Text);
    virtual void OnMessageReceived();
protected:
    uint8_t CsPin = 15;
    uint8_t Dio0Pin = 4;
    uint8_t ResetPin = 0;
    bool bMessageAvailable = false;
    IncomingMessage MessageBuffers[MAX_INC_MESSAGES];
    uint8_t AvailableBufferIndex = 0;
    uint8_t NextIncomingMessageIndex = 0;
protected:
    static constexpr uint8_t SPI_WRITE = 0x80;

    static constexpr uint8_t REG_FIFO = 0x00;
    static constexpr uint8_t REG_OP_MODE = 0x01;
    static constexpr uint8_t REG_FRF_MSB = 0x06;
    static constexpr uint8_t REG_FRF_MID = 0x07;
    static constexpr uint8_t REG_FRF_LSB = 0x08;
    static constexpr uint8_t REG_PA_CONFIG = 0x09;
    static constexpr uint8_t REG_FIFO_RX_CURRENT_ADDR = 0x10;
    static constexpr uint8_t REG_LNA = 0x0C;
    static constexpr uint8_t REG_FIFO_ADDR_PTR = 0x0D;
    static constexpr uint8_t REG_FIFO_TX_BASE_ADDR = 0x0E;
    static constexpr uint8_t REG_FIFO_RX_BASE_ADDR = 0x0F;
    static constexpr uint8_t REG_IRQ_FLAGS = 0x12;
    static constexpr uint8_t REG_FIFO_RX_NB_BYTES = 0x13;
    static constexpr uint8_t REG_MODEM_STAT = 0x18;
    static constexpr uint8_t REG_MODEM_CONFIG1 = 0x1D;
    static constexpr uint8_t REG_MODEM_CONFIG2 = 0x1E;
    static constexpr uint8_t REG_PREAMBLE_MSB = 0x20;
    static constexpr uint8_t REG_PREAMBLE_LSB = 0x21;
    static constexpr uint8_t REG_PAYLOAD_LENGTH = 0x22;
    static constexpr uint8_t REG_MODEM_CONFIG3 = 0x26;
    static constexpr uint8_t REG_DIO_MAPPING1 = 0x40;
    static constexpr uint8_t REG_VERSION = 0x42;

    static constexpr float FXOSC = 32000000.f;
    static constexpr float FSTEP = FXOSC / 524288;

};