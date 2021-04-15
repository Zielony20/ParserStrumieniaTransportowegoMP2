//#pragma once
#include "tsCommon.h"
#include <string>

/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================

class xTS
{
public:
  static constexpr uint32_t TS_PacketLength = 188;
  static constexpr uint32_t TS_HeaderLength = 4;

  static constexpr uint32_t PES_HeaderLength = 6;

  static constexpr uint32_t BaseClockFrequency_Hz         =    90000; //Hz
  static constexpr uint32_t ExtendedClockFrequency_Hz     = 27000000; //Hz
  static constexpr uint32_t BaseClockFrequency_kHz        =       90; //kHz
  static constexpr uint32_t ExtendedClockFrequency_kHz    =    27000; //kHz
  static constexpr uint32_t BaseToExtendedClockMultiplier =      300;
};

//=============================================================================================================================================================================

class xTS_PacketHeader
{
public:
  enum class ePID : uint16_t
  {
    PAT  = 0x0000,
    CAT  = 0x0001,
    TSDT = 0x0002,
    IPMT = 0x0003,
    NIT  = 0x0010, //DVB specific PID
    SDT  = 0x0011, //DVB specific PID
    NuLL = 0x1FFF,
  };

protected:
  //TODO - header fields

  uint8_t SB;
  uint8_t E;
  uint8_t S;
  uint8_t T;
  uint16_t PID;
  uint8_t TSC;
  uint8_t AFC;
  uint8_t CC;


public:
  void     Reset();
  int32_t  Parse(const uint8_t* Input);
  void     Print() const;
  

public:
  //TODO - direct acces to header values
  uint8_t getSB();
  uint8_t getE();
  uint8_t getS();
  uint8_t getT();
  uint16_t getPID();
  uint8_t getTSC();
  uint8_t getAFC();
  uint8_t getCC();
public:
  //TODO
  bool     hasAdaptationField() const;
  bool     hasPayload        () const { /*TODO*/ }
  uint16_t convertFrom8To16(uint8_t hi, uint8_t lo);
};

//=============================================================================================================================================================================

class xTS_AdaptationField
{
protected:
//AF length
uint8_t AFL;
//mandatory fields
uint8_t FLAGS;
uint8_t AFC;
public:
void Reset();
int32_t Parse(const uint8_t* Input, uint8_t AdaptationFieldControl);
void Print() const;
public:
//derrived values
uint32_t getNumBytes() const { }
};

//=============================================================================================================================================================================




int32_t xTS_PacketHeader::Parse(const uint8_t* Input){


xTS_PacketHeader::SB = Input[0];
xTS_PacketHeader::E = (Input[1] & 0x80) ? 1:0;
xTS_PacketHeader::S = (Input[1] & 0x40) ? 1:0;
xTS_PacketHeader::T = (Input[1] & 0x20) ? 1:0;
xTS_PacketHeader::PID = convertFrom8To16((Input[1] & 0x73),Input[2]);
xTS_PacketHeader::TSC = (Input[3] & 0xC0)>>6 ;
xTS_PacketHeader::AFC = (Input[3] & 0x30)>>4 ;
xTS_PacketHeader::CC = Input[3] & 0xF;


}

void xTS_PacketHeader::Reset(){

}
void xTS_PacketHeader::Print() const{

    printf( "SB: %d ", SB );
    printf( "E: %d ", E  );
    printf( "S: %d ", S  );
    printf( "T: %d ", T  );
    printf( "PID: %d ", PID);
    printf( "TSC: %d ", TSC);
    printf( "AFC: %d ", AFC);
    printf( "CC: %d ", CC );

}
uint16_t xTS_PacketHeader::convertFrom8To16(uint8_t hi, uint8_t lo){
    uint16_t result = 0x0000;

    result = hi;
    result = result << 8;
    result |= lo;
    return result;
}

bool xTS_PacketHeader::hasAdaptationField() const{
  if( AFC == 2 or AFC == 3  )
    return true;
  else
    return false;
}

  uint8_t xTS_PacketHeader::getSB(){
  return SB;
}
  uint8_t xTS_PacketHeader::getE(){
    return E;
  }
  uint8_t xTS_PacketHeader::getS(){
    return S;
  }
  uint8_t xTS_PacketHeader::getT(){
    return T;
  }
  uint16_t xTS_PacketHeader::getPID(){
    return PID;
  }
  uint8_t xTS_PacketHeader::getTSC(){
    return TSC;
  }
  uint8_t xTS_PacketHeader::getAFC(){
    return AFC;
  }
  uint8_t xTS_PacketHeader::getCC(){
    return CC;
  }

//=============================================================================================================================================================================


int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl){
  AFC = AdaptationFieldControl;
  if(AdaptationFieldControl == 2 or AdaptationFieldControl == 3){
    AFL = Input[4];
    FLAGS = Input[5];
  }
  else {
        AFL = 0;
        FLAGS = 0;
      }
     

  return 0;
}
void xTS_AdaptationField::Print() const{

  if(AFL!=0){
    printf( "AFL: %d ", AFL );
    printf( "DC: %d ", (FLAGS & 0x80)?1:0  );
    printf( "RA: %d ", (FLAGS & 0x40)?1:0  );
    printf( "SP: %d ", (FLAGS & 0x20)?1:0  );
    printf( "PR: %d ", (FLAGS & 0x10)?1:0  );
    printf( "OR: %d ", (FLAGS & 0x8)?1:0  );
    printf( "SP: %d ", (FLAGS & 0x4)?1:0  );
    printf( "TP: %d ", (FLAGS & 0x2)?1:0  );
    printf( "EX: %d ", (FLAGS & 0x1)?1:0  );
  }
  if(AFC==3){
    printf( "Stuffing: %d ", AFL-1  );
  }
    

}

//=============================================================================================================================================================================

