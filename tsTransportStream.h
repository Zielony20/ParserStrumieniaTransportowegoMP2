//#pragma once
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
  uint32_t header;
  uint8_t SB;
  uint8_t E;
  uint8_t S;
  uint8_t T;
  uint16_t PID;
  uint8_t TSC;
  uint8_t AFC;
  uint8_t CC;


public:
  void     Reset(); /*TODO*/
  int32_t  Parse(const uint8_t* Input);
  void     Print() const;
  

public:
  //TODO - direct acces to header values
  uint8_t getSyncByte();
  uint8_t getTransportErrorIndicator();
  uint8_t getPayloadUnitStartIndicator();
  uint8_t getTransportPriority();
  uint16_t getPID();
  uint8_t getTransportScramblingControl();
  uint8_t getAdaptationFieldControl();
  uint8_t getContinuityCounter();
public:
  //TODO
  bool     hasAdaptationField() const;
  bool     hasPayload        () const { /*TODO*/ }

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
void Reset(); /*TODO*/
int32_t Parse(const uint8_t* Input, uint8_t AdaptationFieldControl);
void Print() const;
public:
//derrived values
    uint32_t getNumBytes() const;
    uint8_t getAdaptationFieldLenght() {
        return AFL;
    }

};

//=============================================================================================================================================================================


class xPES_PacketHeader
{
public:
enum eStreamId : uint8_t
{
eStreamId_program_stream_map = 0xBC,
eStreamId_padding_stream = 0xBE,
eStreamId_private_stream_2 = 0xBF,
eStreamId_ECM = 0xF0,
eStreamId_EMM = 0xF1,
eStreamId_program_stream_directory = 0xFF,
eStreamId_DSMCC_stream = 0xF2,
eStreamId_ITUT_H222_1_type_E = 0xF8,
};
protected:
    //PES packet header
    uint32_t m_PacketStartCodePrefix;
    uint8_t m_StreamId;
    uint16_t m_PacketLength;
    uint16_t m_HeaderLength;
    public:
        void Reset() {};
    int32_t Parse(const uint8_t* Input);
    void Print() const;
    public:
        //PES packet header
        uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
        uint8_t getStreamId() const { return m_StreamId; }
        uint16_t getPacketLength() const { return m_PacketLength; }
        uint8_t getHeaderLength() const { return m_HeaderLength/8; }
};

//=============================================================================================================================================================================


class xPES_Assembler
{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID = 1,
        StreamPackedLost,
        AssemblingStarted,
        AssemblingContinue,
        AssemblingFinished,
    };
protected:
    //setup
    int32_t m_PID;
    //buffer
    uint8_t* m_Buffer;
    uint32_t m_BufferSize;
    uint32_t m_DataOffset;
    //operation
    int8_t m_LastContinuityCounter;
    bool m_Started;
    xPES_PacketHeader m_PESH;
public:
    xPES_Assembler() { Init(136); }
    ~xPES_Assembler() {}
    void Init(int32_t PID);
    eResult AbsorbPacket(const uint8_t* TransportStreamPacket, xTS_PacketHeader* PacketHeader,  xTS_AdaptationField* AdaptationField);
    void PrintPESH() const { m_PESH.Print(); }
    uint8_t getHeaderLenght() const { return m_PESH.getHeaderLength(); }
    uint8_t* getPacket() { return m_Buffer; }
    int32_t getNumPacketBytes() const { return m_DataOffset; }
    void xBufferReset();
protected:
    
    void xBufferAppend(const uint8_t* Data, uint32_t Size);


};

//=============================================================================================================================================================================

