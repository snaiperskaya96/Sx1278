#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>

#include "Sx1278_Arduino.h"

#ifdef ESP8266
Sx1278 RFM;
#else
Sx1278_Arduino RFM(5, 14);
#endif 


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
#endif