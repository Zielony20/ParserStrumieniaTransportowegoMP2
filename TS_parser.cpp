#include <stdio.h>
#include <cstdint>
#include <stdint.h>
#include "tsCommon.h"
#include "tsTransportStream.h"


int main( int argc, char *argv[ ], char *envp[ ])
{

  // TODO - open file
  FILE* pFile;
  pFile = fopen ("example_new.ts","rb");

  if(pFile==NULL){
    exit(1);
  }

  xTS_PacketHeader    TS_PacketHeader;
  xTS_AdaptationField TS_AdaptationField;

  int32_t TS_PacketId = 0;
  while(!feof( pFile ))
  {
    // TODO - read from file
    //const int iRozmiarBufora = 188;
    //uint8_t bufor[iRozmiarBufora];
    //int iOdczytanoBajtow = fread( bufor, 1, 188, pFile );
    //printf( "Odczytano %d bajtow.\n", iOdczytanoBajtow );
    
    const int iRozmiarBufora = 188;
    uint8_t TS_PacketBuffer[iRozmiarBufora];
    int iOdczytanoBajtow = fread( TS_PacketBuffer, 1, 188, pFile );

    

    TS_PacketHeader.Reset();
    TS_PacketHeader.Parse(TS_PacketBuffer);
    TS_AdaptationField.Parse(TS_PacketBuffer,TS_PacketHeader.getAFC());


      printf("%010d ", TS_PacketId);
      TS_PacketHeader.Print();
      TS_AdaptationField.Print();
      printf("\n");
    
    
    if(TS_PacketId>50000)
      break;
    TS_PacketId++;
  }




  fclose (pFile);
  
}