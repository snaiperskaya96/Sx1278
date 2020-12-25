SX1278 
=============

A small driver for the SX1278 and friends (1276/77/78, RFM95/96... anything like that).

Only implements what i need plus a simple API for transmitting and receiving payloads with explicit headers over the LoRa protocol.

I'm aware there are another billion libraries that do the same thing but they are either overloaded with stuff that you don't need and will never use or just badly written, therefore I decided to write my own implementation.

You can find a basic trasmitter and receiver implementation in the Examples folder.

The library should support anything using either the Arduino or the IDF frameworks.

If you are not using platformio you should compile with -DESP32_IDF if you are using IDF or just include Arduino.h before the library's header if you are using Arduino.

It surely lacks bits and bobs but feel free to request for more features or push a PR. 