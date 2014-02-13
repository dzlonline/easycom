//************************************************************************
//  Quick and dirty BK2421 2.4GHz tranceiver chip library for general
//  purpose character based communication
//  Version 1.1
//
//	(C) Dzl Januar 2014
//  http://dzlsevilgeniuslair.blogspot.dk/
//************************************************************************

// Connect BK2421(or compatible e.g. RF24L01 from SparkFun) based radio to:

//Arduino pin 8  to radio CE
//Arduino pin 10 to radio CSN
//Arduino pin 11 to radio MOSI
//Arduino pin 12 to radio MISO
//Arduino pin 13 to radio SCK

#ifndef _EASYCOM
#define _EASYCOM

#include <arduino.h>

#define SET(x,y) (x |=(1<<y))					//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       		// |
#define CHK(x,y) (x & (1<<y))           		// |
#define TOG(x,y) (x^=(1<<y))            		//-+

//-Default registers

const uint8_t PROGMEM initBlock[]=
{
  2,0X20,0X0A,			    //-Power up, receive mode, enable CRC
  2,0X21,0X3F,                      //-Auto ACK all pipes
  2,0X22,0X3F,                      //-Enable all pipes
  2,0X23,0X03,                      //-5 bytes data length
  2,0X24,0X1A,                      //-500µS retransmit rate, 10 times
  2,0x25,0x00,                      //-Channel 0 = 2400 MHz
  2,0X26,0X07,                      //-Max RF power ~3mW
  2,0X27,0X07,                      //-Clear status register
  2,0X28,0X00,                      //-Write to packet counter ??
  2,0X29,0X00,                      //-Write to carrier detect ??
  2,0X2C,0XC3,                      //-Pipe 2 LSB address
  2,0X2D,0XC4,                      //-Pipe 3 LSB address
  2,0X2E,0XC5,                      //-Pipe 4 LSB address
  2,0X2F,0XC6,                      //-Pipe 5 LSB address
  2,0X31,0X08,                      //-8 bytes in pipe 0 payload data
  2,0X32,0X08,                      //-8 bytes in pipe 1 payload data
  2,0X33,0X08,                      //-8 bytes in pipe 2 payload data
  2,0X34,0X08,                      //-8 bytes in pipe 3 payload data
  2,0X35,0X08,                      //-8 bytes in pipe 4 payload data
  2,0X36,0X08,                      //-8 bytes in pipe 5 payload data
  2,0X37,0X00,                      //-Write to FIFO status ??
  2,0X3C,0X3F,                      //-Enable variable payload length for all pipes
  2,0X1D,0X00,                      //-Read Feature register
  2,0X50,0X73,                      //-ACTIVATE: R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK
  2,0X3C,0X3F,			    //-Enable variable payload length for all pipes (again)
  2,0X3D,0X07,			    //-Enable features
  2,0X1D,0X00,			    //-Read features ??
  6,0X2A,0X20,0X30,0X40,0X50,0X60,  //-Set network RX address (default)
  6,0X30,0X20,0X30,0X40,0X50,0X60,  //-Set network TX address (default)
  2,0X07,0X00,	                    //-Read status
  2,0X50,0X53,			    //-Select register bank 1
  5,0X20,0X40,0X4B,0X01,0XE2,	    //-As in datasheet example
  5,0X21,0XC0,0X4B,0X00,0X00,	    //-As in datasheet example
  5,0X22,0XD0,0XFC,0X8C,0X02,	    //-As in datasheet example
  5,0X23,0X99,0X00,0X39,0X21,	    //-As in datasheet example
  5,0X24,0XD9,0X96,0X82,0X1B,	    //-As in datasheet example
  5,0X25,0X24,0X06,0X7F,0XA6,	    //-As in datasheet example
  5,0X2C,0X00,0X12,0X73,0X00,	    //-As in datasheet example
  5,0X2D,0X46,0XB4,0X80,0X00,	    //-As in datasheet example
  5,0X24,0XDF,0X96,0X82,0X1B,	    //-As in datasheet example
  5,0X24,0XD9,0X96,0X82,0X1B,	    //-As in datasheet example
  2,0X07,0X00,                      //-Read status
  2,0X50,0X53,			    //-Select register bank 1
  2,0x20,0x0b,			    //-PRX mode
  2,0X27,0X7F,
  1,0XE1,							//-Flush TX
  1,0XE2,							//-Flush RX
  0
};

const uint8_t PROGMEM flushRX[]=
{
  1,0xE2,
  0
};

const uint8_t PROGMEM flushTX[]=
{
  1,0xE1,
  0
};

const uint8_t PROGMEM PRX[]=
{
  2,0x20,0x7B,
  0
};

const uint8_t PROGMEM PTX[]=
{
  2,0x20,0x7A,
  0
};

const uint8_t PROGMEM LENGTH[]=
{
  2,0x60,0x00,
  0
};

const uint8_t PROGMEM STATUS[]=
{
  2,0x00,0xff,
  0
};

const uint8_t PROGMEM CLR_STATUS[]=
{
  2,0x27,0x70,
  0
};

class easyCom
{
private:

  unsigned char rxchan;

  unsigned char txbuf[32];
  unsigned char rxbuf[32];
  unsigned char cmdbuf[16];
  unsigned char rxptr;
  unsigned char txptr;
  unsigned char readptr;

  void offline()
  {
    delayMicroseconds(10);
    CLR(PORTB,0);        //-CLR CE
    delayMicroseconds(10);
  }

  void online()
  {
    delayMicroseconds(10);
    SET(PORTB,0);        //-SET CE
    delayMicroseconds(10);
  }

  void block(const uint8_t PROGMEM *p)
  {
    unsigned char rxp=0;
    unsigned char len=pgm_read_byte(&p[0]);
    unsigned char adr=1;
    while(len)
    {
      CLR(PORTB,2);
      delayMicroseconds(10);
      rxp=0;
      for(unsigned char i=0;i<len;i++)
      {
        unsigned char b=pgm_read_byte(&p[adr++]);
        SET(SPSR,SPIF);
        SPDR=b;
        while(!CHK(SPSR,SPIF))
          asm("nop");
        cmdbuf[rxp++]=SPDR;
      }
      rxp=0;
      delayMicroseconds(10);
      SET(PORTB,2);
      len=pgm_read_byte(&p[adr++]);
      delayMicroseconds(100);
    }
  }

  void sendCmd(unsigned char n)
  {
    delayMicroseconds(10);
    CLR(PORTB,2);
    delayMicroseconds(10);

    for(unsigned char i=0;i<n;i++)
    {
      SET(SPSR,SPIF);
      SPDR=cmdbuf[i];
      while(!CHK(SPSR,SPIF))
        asm("nop");
      cmdbuf[i]=SPDR;
    }
    delayMicroseconds(10);
    SET(PORTB,2);
    delayMicroseconds(10);
  }

  unsigned char getStatus()
  {
    block(STATUS);
    return cmdbuf[0];  //-Get status
  }
public:

  void begin()
  {
    SET(PORTB,2);
    SET(DDRB,2);      //SS=output
    CLR(PORTB,0);
    SET(DDRB,0);      //SE=output
    SET(DDRB,5);      //SCK
    SET(DDRB,3);      //MOSI
    SPCR=0b01010011;  //Master

    delay(500);
    block(initBlock);
    delay(500);
    online();
    block(flushTX);
    block(PRX);
  }

  easyCom()
  {
    rxchan=0;

    rxptr=0;
    txptr=0;
    readptr=0;
  }

  void write(unsigned char c)
  {
    txbuf[txptr++]=c;
    if(txptr==31)
      transmit();
  }

  unsigned char transmit()
  {
    block(flushTX);

    delayMicroseconds(10);
    CLR(PORTB,2);
    delayMicroseconds(10);

    SET(SPSR,SPIF);
    SPDR=0xA0;
    while(!CHK(SPSR,SPIF))
      asm("nop");

    for(unsigned char i=0;i<txptr;i++)
    {
      SET(SPSR,SPIF);
      SPDR=txbuf[i];
      while(!CHK(SPSR,SPIF))
        asm("nop");
    }
    txptr=0;
    delayMicroseconds(10);
    SET(PORTB,2);
    delayMicroseconds(10);

    offline();
    block(PTX);
    online();

    unsigned char c=0;
    while(!c)
      c=getStatus()&0xf0;

    offline();
    block(PRX);
    online();
    if(c&0x20)  //ACK
      return 1;
    return 0;
  }

  unsigned char readFIFO()
  {
    block(LENGTH);
    unsigned char len=cmdbuf[1];
    //    unsigned char rxptr=0;
    delayMicroseconds(10);
    CLR(PORTB,2);
    delayMicroseconds(10);

    SET(SPSR,SPIF);
    SPDR=0x61;
    while(!CHK(SPSR,SPIF))
      asm("nop");
    rxbuf[rxptr]=SPDR;    //Dummy read status

    for(unsigned char i=0;i<len;i++)
    {
      SET(SPSR,SPIF);
      SPDR=0x00;
      while(!CHK(SPSR,SPIF))
        asm("nop");
      rxbuf[rxptr++]=SPDR;
    }
    delayMicroseconds(10);
    SET(PORTB,2);
    delayMicroseconds(10);
    block(flushRX);
    return len;
  }

  unsigned char available()
  {
    unsigned char s=getStatus();
    if(s&0xf0)
    {
      block(CLR_STATUS);
      if(s&0x40)  //-Data avail
        readFIFO();
    }
    return rxptr-readptr;
  }

  unsigned char read()
  {
    if(readptr==rxptr)
      return 0;
    unsigned char c=rxbuf[readptr++];
    if(readptr==rxptr)
      readptr=rxptr=0;
    return c;
  }

  unsigned char *getBuffer()
  {
    unsigned char*p=&rxbuf[readptr];
    readptr=rxptr=0;
    return p;
  }

  void setID(unsigned char *ID)
  {
    offline();
    cmdbuf[0]=0x2A;
    cmdbuf[1]=ID[0];
    cmdbuf[2]=ID[1];
    cmdbuf[3]=ID[2];
    cmdbuf[4]=ID[3];
    cmdbuf[5]=ID[4];
    sendCmd(6);

    cmdbuf[0]=0x30;
    cmdbuf[1]=ID[0];
    cmdbuf[2]=ID[1];
    cmdbuf[3]=ID[2];
    cmdbuf[4]=ID[3];
    cmdbuf[5]=ID[4];
    sendCmd(6);
    online();
  }
  //  Channel =2400 + channel [MHz] ISM =2400-2500 MHz
  void setChannel(unsigned char chan)
  {
    if(chan>99)  //-No frequencies above 2500 MHz
      chan=99;   //
    rxchan=chan;
    offline();
    cmdbuf[0]=0x25;
    cmdbuf[1]=chan;
    sendCmd(2);
    online();
  }

  unsigned char getChannel()
  {
    return rxchan;
  }
};

#endif


