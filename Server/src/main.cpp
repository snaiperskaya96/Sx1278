#include <Arduino.h>
#include <SPI.h>

#include "RFM95Arduino.h"
#include "Util.h"

RFM95Arduino RFM(5, 14);

void setup()
{
  Serial.begin(115200);

  RFM.Init();
  RFM.Receive();

  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
  Serial.println("Server");
}

const uint32_t Delay = 1;
uint32_t LedTime = 0;
bool bLedOn = false;

void loop()
{
  if (RFM.IsMessageAvailable())
  {
    if (IncomingMessage *Message = RFM.GetNextIncomingMessage())
    {
      uint32_t MessageId = 0;
      memcpy(&MessageId, Message->Buffer, sizeof(uint32_t));
      Serial.print("Message received. Id: ");
      Serial.println(MessageId);
      bLedOn = true;
    }
  }

  digitalWrite(22, bLedOn ? HIGH : LOW);
  delay(Delay);
  if (bLedOn)
  {
    LedTime += Delay;
    if (LedTime >= 200)
    {
      LedTime = 0;
      bLedOn = false;
    }
  }
}