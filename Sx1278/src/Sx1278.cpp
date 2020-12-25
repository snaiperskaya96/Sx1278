#include "Sx1278.h"
#include <cassert>
#include <stdio.h>
#include <cstring>
#include "Util.h"

void ReplaceInString(char ReplaceWhat, char WithWhat, char *String, size_t StringLen)
{
    for (size_t I = 0; I < StringLen; I++)
    {
        if (String[I] == ReplaceWhat)
        {
            String[I] = WithWhat;
        }
    }
}

void Sx1278::Init()
{
    Log("Sx1278::Init");
    PinMode(ResetPin, EIOMode::Output);
    PinMode(Dio0Pin, EIOMode::Input);
    PinMode(CsPin, EIOMode::Output);

    DigitalWrite(ResetPin, 1);

    assert(ReadRegister(REG_VERSION) == 0x12 && "Unmatching LoRa magic");

    Reset();

    SetMode(EOpMode::Sleep);

    SetupDIO0Interrupt();

    SetFrequency(434.0f);
    SetPreamble(10);

    // 125 Khz BW | 4/5 error coding rate | Explicit header
    //WriteRegister(REG_MODEM_CONFIG1, (0x07 << 4) | (0x1 << 1) | 0x0);
    //explicit header
    WriteRegister(REG_MODEM_CONFIG1, 0x72);
    WriteRegister(REG_MODEM_CONFIG2, 0x74);
    WriteRegister(REG_MODEM_CONFIG3, 0x04);

    WriteRegister(REG_LNA, ReadRegister(REG_LNA) | 0x03);

    WriteRegister(REG_FIFO_TX_BASE_ADDR, 0x0);
    WriteRegister(REG_FIFO_RX_BASE_ADDR, 0x0);

    // enable PA, default power - this is quite important, we don't use the antenna otherwise
    WriteRegister(REG_PA_CONFIG, 0x0f | 0x40 | 0x80);

    SetMode(EOpMode::Stdby);

    Log("Sx1278 Initialised");
}

IncomingMessage* Sx1278::GetNextIncomingMessage()
{
    IncomingMessage* NextMessage = &MessageBuffers[NextIncomingMessageIndex];
    if (NextMessage && !NextMessage->bRead)
    {
        NextMessage->bRead = true;
        NextIncomingMessageIndex = (NextIncomingMessageIndex + 1) >= GetNumMessageBuffers() ? 0 : (NextIncomingMessageIndex + 1);
        bMessageAvailable = !MessageBuffers[NextIncomingMessageIndex].bRead;
        return NextMessage;
    }

    return nullptr;
}

void Sx1278::OnMessageReceived()
{
    const uint8_t IncSize = ReadRegister(REG_FIFO_RX_NB_BYTES);
    WriteRegister(REG_FIFO_ADDR_PTR, ReadRegister(REG_FIFO_RX_CURRENT_ADDR));

    IncomingMessage& Message = MessageBuffers[AvailableBufferIndex++];

    Message.bRead = false;
    Message.Size = IncSize;
    Read(REG_FIFO, IncSize, Message.Buffer);

    bMessageAvailable = true;

    // Cycle buffer 
    if (AvailableBufferIndex >= GetNumMessageBuffers())
    {
        AvailableBufferIndex = 0;
    }

    if (true)
    {
        static char TmpBuf[512] = {0x0};
        const char Desc[] = "Received %d bytes: %s";
        sprintf(TmpBuf, Desc, IncSize, Message.Buffer);
        ReplaceInString(0x0, 0x1, TmpBuf, 512);
        TmpBuf[IncSize + sizeof(Desc)] = 0x0;
        Log(TmpBuf);
    }
}

void Sx1278::Receive(uint32_t Timeout)
{
    assert(Timeout == 0 && "Timed receive not implemented");
    EOpMode CurrentMode = GetMode();

    while (!(CurrentMode == EOpMode::Sleep || CurrentMode == EOpMode::Stdby))
    {
        SetMode(EOpMode::Stdby);
        Wait(1);
        CurrentMode = GetMode();
    }

    WriteRegister(REG_FIFO_ADDR_PTR, 0);
    SetMode(EOpMode::RxContinuous);
    Log("Waiting for messages");
}

void Sx1278::SetMode(EOpMode Mode)
{
    const uint8_t LoraLongRange = 0x80; // 10000000;

    WriteRegister(REG_OP_MODE, LoraLongRange | (uint8_t)Mode);
    uint32_t Delay = 10;
    switch (Mode)
    {
    case EOpMode::Tx:
        // Enable TxDone interrupt
        WriteRegister(REG_DIO_MAPPING1, 0x40);
        break;
    case EOpMode::RxSingle:
    case EOpMode::RxContinuous:
        // Enable RxDone interrupt
        WriteRegister(REG_DIO_MAPPING1, 0x00);
        break;
    default:
        break;
    }
    Wait(Delay);
    assert(GetMode() == Mode);
}

EOpMode Sx1278::GetMode()
{
    return (EOpMode)(ReadRegister(REG_OP_MODE) & 0x7);
}

bool Sx1278::CanTransmit()
{
    const EOpMode Mode = GetMode();
    return Mode == EOpMode::Sleep || Mode == EOpMode::Stdby;
}

void Sx1278::TransmitData(const uint8_t *Buffer, size_t BufferLength)
{
    SetMode(EOpMode::Stdby);

    WriteRegister(REG_FIFO_ADDR_PTR, 0); //ReadRegister(REG_FIFO_TX_BASE_ADDR));
    for (int i = 0; i < BufferLength; i++)
    {
        WriteRegister(REG_FIFO, Buffer[i]);
    }

    // Debug stuff
    if (false)
    {
        const uint8_t CurAddress = ReadRegister(REG_FIFO_ADDR_PTR);
        char Buf[64] = {0x0};
        sprintf(Buf, "Current address is %d should be %d", CurAddress, (uint8_t)BufferLength);
        Log(Buf);
    }
    /////

    WriteRegister(REG_PAYLOAD_LENGTH, (uint8_t)BufferLength);
    SetMode(EOpMode::Tx);
}

void Sx1278::OnDIO0Interrupt()
{
    const uint8_t IrqFlags = ReadRegister(REG_IRQ_FLAGS);
    WriteRegister(REG_IRQ_FLAGS, 0xFF); // Clear IRQ flags

    if (IrqFlags & (uint8_t)EIrqFlags::TxDone)
    {
        //Log("TxDone");
    }
    if (IrqFlags & (uint8_t)EIrqFlags::RxDone)
    {
        if (IrqFlags & (uint8_t)EIrqFlags::ValidHeader)
        {
            OnMessageReceived();
        }
        else
        {
            Log("Discarding incoming packet with invalid header");
        }
    }

    if (false)
    {
        char Tmp[32] = {0x0};
        sprintf(Tmp, "Irq: " BYTE_TO_BINARY_PATTERN "", BYTE_TO_BINARY(IrqFlags));
        Log(Tmp);
    }
}

void Sx1278::SetFrequency(float Frequency)
{
    const uint32_t Freq = (Frequency * 1000000.f) / FSTEP;
    WriteRegister(REG_FRF_MSB, (Freq >> 16) & 0xff);
    WriteRegister(REG_FRF_MID, (Freq >> 8) & 0xff);
    WriteRegister(REG_FRF_LSB, Freq & 0xff);
}

void Sx1278::Log(const char *Text)
{
}

uint8_t Sx1278::ReadRegister(uint8_t Address)
{
    static uint8_t OutBuffer = 0;
    Read(Address, 1, &OutBuffer);
    return OutBuffer;
}

void Sx1278::WriteRegister(uint8_t Address, uint8_t Value)
{
    Write(Address, &Value, 1);
}

void Sx1278::Reset()
{
    DigitalWrite(ResetPin, 0);
    Wait(1); // It's meant to be 0.1ms but hey
    DigitalWrite(ResetPin, 1);
    Wait(5);
}

void Sx1278::SetPreamble(uint16_t Preamble)
{
    uint8_t Lsb = Preamble & 0xF;
    uint8_t Msb = (Preamble >> 4) & 0xF;

    WriteRegister(REG_PREAMBLE_LSB, Lsb);
    WriteRegister(REG_PREAMBLE_MSB, Msb);

    const uint16_t NewPreamble = (((uint16_t)ReadRegister(REG_PREAMBLE_MSB)) << 4) | ReadRegister(REG_PREAMBLE_LSB);
    assert(NewPreamble == Preamble);
}
