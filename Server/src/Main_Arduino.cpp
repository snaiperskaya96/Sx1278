#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>

#include "Sx1278_Arduino.h"
#include "Util.h"

#ifdef ESP8266
Sx1278 RFM;
const uint8_t LedPin = D3;
#else
Sx1278_Arduino RFM(5, 14);
const uint8_t LedPin = 22;
#endif 

void setup()
{
  Serial.begin(115200);

  RFM.Init();
  RFM.Receive();

  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, LOW);
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

  digitalWrite(LedPin, bLedOn ? HIGH : LOW);
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
#endif