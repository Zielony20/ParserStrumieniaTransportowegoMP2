#include "tsTransportStream.h"
#include <cstdint>
#include <stdint.h>
#include <cinttypes>
#include <cfloat>
#include <climits>
#include <cstddef>
#include "tsCommon.h"

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================


int32_t xTS_PacketHeader::Parse(const uint8_t* Input) {

    xTS_PacketHeader::SB = Input[0];
    xTS_PacketHeader::E = (Input[1] & 0x80) ? 1 : 0;
    xTS_PacketHeader::S = (Input[1] & 0x40) ? 1 : 0;
    xTS_PacketHeader::T = (Input[1] & 0x20) ? 1 : 0;
    xTS_PacketHeader::PID = convertFrom8To16((Input[1] & 0x1F), Input[2]);
    xTS_PacketHeader::TSC = (Input[3] & 0xC0) >> 6;
    xTS_PacketHeader::AFC = (Input[3] & 0x30) >> 4;
    xTS_PacketHeader::CC = Input[3] & 0xF;
    header = convertFrom8To32(Input[0], Input[1], Input[2], Input[3]);
    return header;
}

void xTS_PacketHeader::Reset() {
    SB, E, S, T, PID, TSC, AFC, CC = 0;
}
void xTS_PacketHeader::Print() const {

    printf("SB: %d ", SB);
    printf("E: %d ", E);
    printf("S: %d ", S);
    printf("T: %d ", T);
    printf("PID: %d ", PID);
    printf("TSC: %d ", TSC);
    printf("AFC: %d ", AFC);
    printf("CC: %d ", CC);

}


bool xTS_PacketHeader::hasAdaptationField() const {
    if ((AFC == 2) || (AFC == 3))
        return true;
    else
        return false;
}

uint8_t xTS_PacketHeader::getSyncByte() {
    return SB;
}
uint8_t xTS_PacketHeader::getTransportErrorIndicator() {
    return (header & 0x800000) ? 1 : 0;
}
uint8_t xTS_PacketHeader::getPayloadUnitStartIndicator() {
    return S;
}
uint8_t xTS_PacketHeader::getTransportPriority() {
    return T;
}
uint16_t xTS_PacketHeader::getPID() {
    return PID;
}
uint8_t xTS_PacketHeader::getTransportScramblingControl() {
    return TSC;
}
uint8_t xTS_PacketHeader::getAdaptationFieldControl() {
    return AFC;
}
uint8_t xTS_PacketHeader::getContinuityCounter() {
    return CC;
}



//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================


int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl) {
    int pointer = 4;
    AFC = AdaptationFieldControl;
    if ((AdaptationFieldControl == 1) || (AdaptationFieldControl == 3)) {
        AFL = Input[pointer++]; //4
        FLAGS = Input[pointer++];  //5
    }

    else {
        AFL = 0;
        FLAGS = 0;
    }

    //Is PCR_FLAG
    if (0b00010000 & FLAGS) {
        PCR = convertFrom8To64(0, 0, Input[++pointer], Input[++pointer],
            Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
    }
    //Is OPCR_FLAG
    if (0b00001000 & FLAGS) {
        OPCR = convertFrom8To64(0, 0, Input[++pointer], Input[++pointer],
            Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
    }


    return 0;
} 
void xTS_AdaptationField::Reset() {
    AFL = 0;
    FLAGS = 0;
}


void xTS_AdaptationField::Print() const {

    if (AFL != 0) {
        printf("AFL: %d ", AFL);
        printf("DC: %d ", (FLAGS & 0x80) ? 1 : 0);
        printf("RA: %d ", (FLAGS & 0x40) ? 1 : 0);
        printf("SP: %d ", (FLAGS & 0x20) ? 1 : 0);
        printf("PR: %d ", (FLAGS & 0x10) ? 1 : 0);
        printf("OR: %d ", (FLAGS & 0x8) ? 1 : 0);
        printf("SP: %d ", (FLAGS & 0x4) ? 1 : 0);
        printf("TP: %d ", (FLAGS & 0x2) ? 1 : 0);
        printf("EX: %d ", (FLAGS & 0x1) ? 1 : 0);
    }
    //if (0b00010000 & FLAGS) { printf("PCR: %d ", PCR); }
    //if (0b00001000 & FLAGS) { printf("OPCR: %d ", OPCR); }

    if (AFC == 3) {
        printf("Stuffing: %d ", AFL - 1);
    }


}
uint32_t xTS_AdaptationField::getNumBytes() const { 
    if (AFL == 0) 
        return 0;
    else
        return AFL+1;
}
uint8_t xTS_AdaptationField::getAdaptationFieldLenght() {
    return AFL;
}


//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================


int32_t xPES_PacketHeader::Parse(const uint8_t* Input) {
    m_PacketStartCodePrefix = convertFrom8To24(Input[0], Input[1], Input[2]);
    m_StreamId = Input[3];
    m_PacketLength = convertFrom8To16(Input[4],Input[5]);
    if(m_PacketLength==0) 
        printf("PES packet length is neither specified nor bounded and is allowed only in PES packets whose payload consists of bytes from a video elementary stream contained in transport stream packets.");
    m_HeaderLength = 6;
    //if (m_StreamId == 189 || (m_StreamId & 0b11100000) == 0b11000000 || (m_StreamId & 0b11110000) == 0b11100000)
    //    m_HeaderLength = 14; //+=Input[8];
    
    //else if (m_PESH.getStreamId() == 0b10111110 || m_PESH.getStreamId() == 0b10111111) {}
   
    if (m_StreamId != 0xBC &&
        m_StreamId != 0xBE &&
        m_StreamId != 0xBF &&
        m_StreamId != 0xF0 &&
        m_StreamId != 0xF1 &&
        m_StreamId != 0xFF &&
        m_StreamId != 0xF2 &&
        m_StreamId != 0xF8)
        m_HeaderLength = 14;//Input[8];
        
    //m_HeaderLength = Input[8];

    return m_PacketStartCodePrefix;
}
void xPES_PacketHeader::Print() const {
    printf("PSCP: %d ", m_PacketStartCodePrefix);
    printf("SID: %d ", m_StreamId);
    printf("PacketLength: %d ", m_PacketLength);
};
uint8_t xPES_PacketHeader::getHeaderLength() const { return m_HeaderLength; }
uint16_t xPES_PacketHeader::getPacketLength() const { return m_PacketLength; }
uint32_t xPES_PacketHeader::getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
uint8_t xPES_PacketHeader::getStreamId() const { return m_StreamId; }

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================

void xPES_Assembler::Init(int32_t PID) {
    m_PID = PID;
    m_Buffer = new uint8_t[10000];
    m_BufferSize=0;
};

void xPES_Assembler::PrintPESH() const { m_PESH.Print(); }

uint8_t xPES_Assembler::getHeaderLenght() const { return m_PESH.getHeaderLength(); }

uint8_t* xPES_Assembler::getPacket() { return m_Buffer; }

int32_t xPES_Assembler::getNumPacketBytes() const { return m_BufferSize; }


//!!!!!!!!!!!!!!!!!!!!!!
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(
    const uint8_t*          TransportStreamPacket,
    xTS_PacketHeader*       PacketHeader,
    xTS_AdaptationField*    AdaptationField) {
    
    uint8_t TS_AdaptationLenght = 0;
    if (PacketHeader->hasAdaptationField()) {
        TS_AdaptationLenght = AdaptationField->getNumBytes();
    }
    uint32_t tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;
    //printf("Rozmiar PES: %d ", tempSize);
    xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
    //printf("Offset: %d ", m_BufferSize);


    if (PacketHeader->getPayloadUnitStartIndicator()) {
        m_PESH.Parse(m_Buffer);
        PrintPESH();
        return eResult::AssemblingStarted;
    }
    else if(PacketHeader->hasAdaptationField()){
        
        return eResult::AssemblingFinished;
    }
    else {
        return eResult::AssemblingContinue;
    }

}
void xPES_Assembler::xBufferReset() {
    
    m_BufferSize = 0;

}
void xPES_Assembler::xBufferAppend(const uint8_t* Data, uint32_t Size) {

        uint32_t sizeToSkip = xTS::TS_PacketLength - Size;
        //printf("size to skip: %d", sizeToSkip);

        for (int i = 0; i <  Size; i++) {
            m_Buffer[m_BufferSize + i] = Data[sizeToSkip + i];
            //printf(" %d ", m_Buffer[m_BufferSize + i]);
        }
        m_BufferSize += Size;
}





