#include "tsTransportStream.h"
#include <cstdint>
#include <stdint.h>
#include <cinttypes>
#include <cfloat>
#include <climits>
#include <cstddef>
//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

xTS_PacketHeader::xTS_PacketHeader(){

}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input){


xTS_PacketHeader::SB = Input[0];
xTS_PacketHeader::E = (Input[1] & 0x80) ? 1:0;
xTS_PacketHeader::S = (Input[1] & 0x40) ? 1:0;
xTS_PacketHeader::T = (Input[1] & 0x20) ? 1:0;
xTS_PacketHeader::PID = convertFrom8To16((Input[1] & 0x73),Input[2]);
xTS_PacketHeader::TSC = Input[3] & 0xC4 ? 1 : 0;
xTS_PacketHeader::AFC = Input[3] & 0x30 ? 1 : 0;
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




//=============================================================================================================================================================================
