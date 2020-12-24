#include <Arduino.h>
#include <SPI.h>

#include "RFM95Arduino.h"

RFM95Arduino RFM(5, 14);

void setup()
{
  Serial.begin(115200);
  
  RFM.Init();
  Serial.println("Client");
}

void loop()
{
  static uint32_t MessageId = 0;
  if (RFM.CanTransmit())
  {
    RFM.TransmitData((const uint8_t*)&MessageId, sizeof(uint32_t));
    MessageId++;
  }
  
  delay(1000);
}