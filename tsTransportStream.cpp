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
    //xTS_PacketHeader::E = (Input[1] & 0x80) ? 1 : 0;
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
    AFC = AdaptationFieldControl;
    if ((AdaptationFieldControl == 2) || (AdaptationFieldControl == 3)) {
        AFL = Input[4];
        FLAGS = Input[5];
    }
    else {
        AFL = 0;
        FLAGS = 0;
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
    if (AFC == 3) {
        printf("Stuffing: %d ", AFL - 1);
    }


}
uint32_t xTS_AdaptationField::getNumBytes() const { 
    uint32_t temp = AFL;
    return (temp + 1)/8+1;
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================


int32_t xPES_PacketHeader::Parse(const uint8_t* Input) {
    m_PacketStartCodePrefix = convertFrom8To24(Input[0], Input[1], Input[2]);
    m_StreamId = Input[3];
    m_PacketLength = convertFrom8To16(Input[4],Input[5]);
    m_HeaderLength = Input[8];


    return m_PacketStartCodePrefix;
}
void xPES_PacketHeader::Print() const {
    printf("PSCP: %d ", m_PacketStartCodePrefix);
    printf("SID: %d ", m_StreamId);
    printf("PacketLength: %d ", m_PacketLength);
};



//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================
void xPES_Assembler::Init(int32_t PID) {
    m_PID = PID;
    m_Buffer = new uint8_t[10000];
    m_BufferSize=0;
    m_DataOffset=0;
};

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket,
    xTS_PacketHeader* PacketHeader, xTS_AdaptationField* AdaptationField) {
    
    
    uint32_t tempSize = 188 - 4 - AdaptationField->getNumBytes();

    xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
     


    if (PacketHeader->getPayloadUnitStartIndicator() == 1) {
        xBufferReset();
        m_PESH.Parse(m_Buffer);
        //PrintPESH();
        m_DataOffset = m_PESH.getPacketLength();
        return eResult::AssemblingStarted;
    }
    else if(PacketHeader->hasAdaptationField()){
        if(m_PESH.getStreamId()==189 || 5>> m_PESH.getStreamId() == 0b110 || 4 >> m_PESH.getStreamId() == 0b1110 )
        m_DataOffset += getHeaderLenght();
        else if(m_PESH.getStreamId() == 0b10111110 || m_PESH.getStreamId() == 0b10111111) {
            m_DataOffset += 6;
        }


        return eResult::AssemblingFinished;
    }
    else {
        return eResult::AssemblingContinue;
    }

}
void xPES_Assembler::xBufferReset() {
    
    m_DataOffset = 0;
    m_BufferSize = 0;

}
void xPES_Assembler::xBufferAppend(const uint8_t* Data, uint32_t Size) {

        uint32_t sizeToSkip = xTS::TS_PacketLength - Size + 1;
        

        for (int i = 0; i <  Size; i += 1) {
            m_Buffer[m_BufferSize + i] = Data[sizeToSkip + i];
            printf(" %d ", m_Buffer[m_BufferSize + i]);
        }
        m_BufferSize += Size;
        
}





