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
    printf("TS : ");
    printf("SB=%d ", SB);
    printf("E=%d ", E);
    printf("S=%d ", S);
    printf("T=%d ", T);
    printf("PID=%d ", PID);
    printf("TSC=%d ", TSC);
    printf("AFC=%d ", AFC);
    printf("CC=%d ", CC);

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
        Stuffing = AFL - 1;
    }

    else {
        AFL = 0;
        FLAGS = 0;
        Stuffing = 0;
    }
    
    DC = (FLAGS & 0x80) >> 7;
    RA = (FLAGS & 0x40) >> 6;
    SP = (FLAGS & 0x20) >> 5;
    PR = (FLAGS & 0x10) >> 4; 
    OR = (FLAGS & 0x08) >> 3;  
    SPF = (FLAGS & 0x04) >> 2;
    TP = (FLAGS & 0x02) >> 1;
    EX = FLAGS & 0x01;


    //Is PCR_FLAG
    if (PR) {

        PCR_base = convertFrom8To64(0, 0, 0,
            Input[pointer], Input[pointer + 1], Input[pointer + 2], Input[pointer + 3], Input[pointer + 4]);
        pointer += 4;

        PCR_base >>= 7;

        PR_reserved = (Input[pointer] & 0x7E);
        PR_reserved >>= 1;
        PCR_extension = Input[pointer] & 0x1;
        PCR_extension = convertFrom8To16(PCR_extension, Input[pointer + 1]);
        pointer += 1;
        Stuffing -= 6;
    }
    //Is OPCR_FLAG
    if (OR) {
        OPCR_base = convertFrom8To64(0, 0, 0,
            Input[pointer], Input[pointer + 1], Input[pointer + 2], Input[pointer + 3], Input[pointer + 4]);
        pointer += 4;

        PCR_base >>= 7;

        OR_reserved = (Input[pointer] & 0x7E);
        OR_reserved >>= 1;
        OPCR_extension = Input[pointer] & 0x1;
        OPCR_extension = convertFrom8To16(PCR_extension, Input[pointer + 1]);
        pointer += 1;
        Stuffing -= 6;
    }
    if (SPF) {
        Stuffing -= 1;
    }
    if (TP) {
        Stuffing -= 1;
    }
    if (EX) {
        Stuffing -= 2;
    }

    return 0;
}
void xTS_AdaptationField::Reset() {
    AFL = 0;
    FLAGS = 0;
    PCR_base = 0;
    PR_reserved = 0;
    PCR_extension = 0;
    OPCR_base = 0;
    OR_reserved = 0;
    OPCR_extension = 0;
    DC, RA, SP, PR, OR, SPF, TP, EX = 0;
}

void xTS_AdaptationField::Print() const {

    if (AFL != 0) {
        printf("AF : ");
        printf("AFL=%d ", AFL);
        printf("DC=%d ", (DC) ? 1 : 0);
        printf("RA=%d ", (RA) ? 1 : 0);
        printf("SP=%d ", (SP) ? 1 : 0);
        printf("PR=%d ", (PR) ? 1 : 0);
        printf("OR=%d ", (OR) ? 1 : 0);
        printf("SP=%d ", (SP) ? 1 : 0);
        printf("TP=%d ", (TP) ? 1 : 0);
        printf("EX=%d ", (EX) ? 1 : 0);
    }
    if (PR) { printf("PCR=%d ", (PCR_base * xTS::BaseToExtendedClockMultiplier) + PCR_extension); }
    if (OR) { printf("OPCR=%d ", (OPCR_base * xTS::BaseToExtendedClockMultiplier) + OPCR_extension); }

    if (AFC == 3) {
        printf("Stuffing: %d ", Stuffing);
    }


}
uint32_t xTS_AdaptationField::getNumBytes() const {
    if (AFL == 0)
        return 0;
    else
        return AFL + 1;
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
    m_PacketLength = convertFrom8To16(Input[4], Input[5]);
    if (m_PacketLength == 0)
        // printf("PES packet length is neither specified nor bounded and is allowed only in PES packets whose payload consists of bytes from a video elementary stream contained in transport stream packets.");
        m_HeaderLength = 6;


    if (
        m_StreamId != 0xBC &&
        m_StreamId != 0xBE &&
        m_StreamId != 0xBF &&
        m_StreamId != 0xF0 &&
        m_StreamId != 0xF1 &&
        m_StreamId != 0xFF &&
        m_StreamId != 0xF2 &&
        m_StreamId != 0xF8) {
        m_HeaderLength = 9;


        PTS_DTS_flags =  (Input[7] & 0b11000000)>>6;

        if ((Input[7] & 0b11000000) == 0b11000000) {

            m_HeaderLength += 10;

            PTS = Input[9] & 0b1110;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0b11111110;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;


            DTS = Input[14] & 0b1110;
            DTS <<= 7;
            DTS += Input[15];
            DTS <<= 8;
            DTS += Input[16] & 0b11111110;
            DTS <<= 7;
            DTS += Input[17];
            DTS <<= 7;
            DTS += Input[18] >> 1;

            
        }
        else if ((Input[7] & 0b10000000) == 0b10000000) {
            m_HeaderLength += 5;
            
            PTS = Input[9] & 0b1110;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0b11111110;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13]>>1;
            
        }

        //ESCR_flag == '1'
        if (Input[7] & 0b00100000) {
            m_HeaderLength += 6;
        }
        //ES_rate_flag == '1'
        if (Input[7] & 0b00010000) {
            m_HeaderLength += 3;
        }
        //DSM_trick_mode_flag == '1'
        if (Input[7] & 0b00001000) {
            m_HeaderLength += 0;
        }
        //additional_copy_info_flag == '1'
        if (Input[7] & 0b00000100) {
            m_HeaderLength += 1;
        }
        //PES_CRC_flag == '1')
        if (Input[7] & 0b00000010) {
            m_HeaderLength += 2;
        }
        //PES_extension_flag == '1'
        if (Input[7] & 0b00000001) {
            int point = m_HeaderLength;
            m_HeaderLength += 1;
            //PES_private_data_flag == '1' -> 128
            if (Input[point] & 0b10000000) {
                m_HeaderLength += 16;
            }
            //pack_header_field_flag == '1' -> 8
            if (Input[point] & 0b01000000) {
                m_HeaderLength += 1;
            }
            //program packet sequence counter flag
            if (Input[point] & 0b00100000) {
                m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0b00010000) {
                m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0b00000001) {
                m_HeaderLength += 2;
            }

        }

    }

    return m_PacketStartCodePrefix;
}

void xPES_PacketHeader::Print() const {
    printf("PES : ");
    printf("PSCP=%d ", m_PacketStartCodePrefix);
    printf("SID=%d ", m_StreamId);
    printf("PacketLength=%d ", m_PacketLength);
    if (PTS_DTS_flags == 0b10)
        printf("PTS=%d ", PTS);

    if (PTS_DTS_flags == 0b11){
            printf("PTS=%d ", PTS);
            printf("DTS=%d ", DTS);
        }
};

void xPES_PacketHeader::Reset() {
    m_PacketStartCodePrefix, m_StreamId, m_PacketLength, m_HeaderLength, PTS_DTS_flags = 0;
    PTS, DTS = 0;
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
    m_Buffer = new uint8_t[100000];
    m_BufferSize = 0;
};

void xPES_Assembler::PrintPESH() const { m_PESH.Print(); }

uint8_t xPES_Assembler::getHeaderLenght() const { return m_PESH.getHeaderLength(); }

uint8_t* xPES_Assembler::getPacket() { return m_Buffer; }

int32_t xPES_Assembler::getNumPacketBytes() const { return m_BufferSize; }


//!!!!!!!!!!!!!!!!!!!!!!
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(
                                            const uint8_t* TransportStreamPacket,
                                            xTS_PacketHeader* PacketHeader,
                                            xTS_AdaptationField* AdaptationField) {


    
    uint8_t TS_AdaptationLenght = 0;
    if (PacketHeader->hasAdaptationField()) {
        TS_AdaptationLenght = AdaptationField->getNumBytes();
    }

    uint32_t tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;
    

    if (PacketHeader->getPayloadUnitStartIndicator()) {

        tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;
        uint32_t sizeToSkip = xTS::TS_PacketLength - tempSize;
        uint8_t* firstPacketBuffer =new uint8_t[188];

        for (int i = 0; i < tempSize; i++) {
            firstPacketBuffer[i] = TransportStreamPacket[sizeToSkip + i];

        }
        m_PESH.Parse(firstPacketBuffer);

        tempSize -= getHeaderLenght();

        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingStarted;
    }
    else if (PacketHeader->hasAdaptationField()) {


        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);

        return eResult::AssemblingFinished;
    }
    else {

        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);

        return eResult::AssemblingContinue;
    }

}
void xPES_Assembler::xBufferReset() {

    m_BufferSize = 0;

}
void xPES_Assembler::xBufferAppend(const uint8_t* Data, uint32_t Size) {

    uint32_t sizeToSkip = xTS::TS_PacketLength - Size;
    

    for (int i = 0; i < Size; i++) {
        m_Buffer[m_BufferSize + i] = Data[sizeToSkip + i];
        
    }
    m_BufferSize += Size;
}
