#include "easycom.h"

// Serial bridge example:
// Transmit any characters arriving at serial port via radio.
// Sends any data arriving over radio to serial port.

// Connect BK2421(or compatible e.g. RF24L01 from SparkFun) based radio to:

//Arduino pin 8  to radio CE
//Arduino pin 10 to radio CSN
//Arduino pin 11 to radio MOSI
//Arduino pin 12 to radio MISO
//Arduino pin 13 to radio SCK

easyCom com;  //-Create instance of radio (only one supported )

void setup()
{
  Serial.begin(19200);
  com.begin();
  //Uncomment to change Note ID. Default = 0x20,0x30,0x40,0x50,0x60
/*
  unsigned char noteID[]={0x10,0x20,0x30,0x40,0x50};
  com.setID(noteID);
*/
}

void loop()
{
  while(com.available())
    Serial.write(com.read());

  if(Serial.available())
  {
    while(Serial.available())
      com.write(Serial.read());
    com.transmit();
  }
}
