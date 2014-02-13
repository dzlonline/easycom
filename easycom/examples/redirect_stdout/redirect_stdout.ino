#include "easycom.h"

// Example of redirecting stdout to easyCom radio so that e.g. printf prints to easyCom:

// Connect BK2421(or compatible e.g. RF24L01 from SparkFun) based radio to:

//Arduino pin 8  to radio CE
//Arduino pin 10 to radio CSN
//Arduino pin 11 to radio MOSI
//Arduino pin 12 to radio MISO
//Arduino pin 13 to radio SCK

easyCom com;  //-Create instance of radio (only one supported )

//-Redirecting function in the correct format
int sendchar(char c,FILE* p)
{
  com.write(c);
}

void setup()
{
  Serial.begin(19200);

  fdevopen(sendchar,0);  //-Connect standard out to com
  com.begin();
  //Uncomment to change Note ID. Default = 0x20,0x30,0x40,0x50,0x60
/*
  unsigned char noteID[]={0x10,0x20,0x30,0x40,0x50};
  com.setID(noteID);
*/
}

void loop()
{
  printf("Time is %d seconds \r\n",millis()/1000);
  com.transmit();
  delay(1000);
}
