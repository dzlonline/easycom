#include "easycom.h"

// Example for reading buffers from easycom. Useful for receiving structured buffers

// Connect BK2421(or compatible e.g. RF24L01 from SparkFun) based radio to:

//Arduino pin 8  to radio CE
//Arduino pin 10 to radio CSN
//Arduino pin 11 to radio MOSI
//Arduino pin 12 to radio MISO
//Arduino pin 13 to radio SCK

easyCom com;  //-Create instance of radio (only one supported )

void setup()
{
  Serial.begin(115200);  //-Need to be fast for this example
  com.begin();

  //-I tested this using a HCD handset sending 8 byte buffers (your application may be different)
  unsigned char noteID[]={0x65,0x65,0x65,0x65,0x65};//-I used this ID for getting the ID from the handset. Handset returns a,b,c,d,0x56,0xaa,0x40,0x00 ID to use is then: a,b,c,d,0x1C
  //unsigned char noteID[]={0x69,0x80,0x55,0x11,0xc1}; //-My handset had this ID
  com.setID(noteID);
  com.setChannel(0x3C);  //-The HCD base channel (you may choose something different)
}

void loop()
{
  unsigned char n=com.available();        //-Get buffer length
  if(n)                                   //-If anything available we _must_ call getBuffer() or read the individual bytes using read() 
  {
    unsigned char *p=com.getBuffer();    //-Note buffer only available until next com.available() call    
    Serial.print(n);                     //-Print buffer length
    Serial.print(" ");
    for(unsigned char i=0;i<n;i++)       //-Print bytes
    {
      Serial.print(p[i],HEX);
      Serial.print(",");
    }
    Serial.println();
  }
}





