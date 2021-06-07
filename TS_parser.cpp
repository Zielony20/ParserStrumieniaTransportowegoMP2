#include <stdio.h>
#include <cstdint>
#include <stdint.h>
#include "tsTransportStream.h"


int main(int argc, char* argv[], char* envp[])
{
    
    FILE* TransportStreamFile = fopen("example_new.ts", "rb");


    if (TransportStreamFile == NULL) {
        printf("wrong file name\n");
        return EXIT_FAILURE;
    }

    FILE* AudioMP2 = fopen("PID136.mp2", "wb");
    FILE* Video264 = fopen("PID174.264", "wb");
    
    uint8_t TS_PacketBuffer[xTS::TS_PacketLength];
    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_PacketAdaptationField;


    xPES_Assembler PES_Assembler136; 
    xPES_Assembler PES_Assembler174;
    PES_Assembler136.Init(136);
    PES_Assembler174.Init(174);

    int32_t TS_PacketId = 0;

    
    while (!feof(TransportStreamFile))
    {
        size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);


        if (NumRead != xTS::TS_PacketLength) { break; }
        
        TS_PacketHeader.Reset();
        TS_PacketHeader.Parse(TS_PacketBuffer);
        TS_PacketAdaptationField.Reset();
        uint8_t temp = TS_PacketHeader.getSyncByte();
        if (TS_PacketHeader.getSyncByte() == 'G' && (TS_PacketHeader.getPID() == 136  || TS_PacketHeader.getPID() == 174))//  && TS_PacketId<50000)
        {
            if (TS_PacketHeader.hasAdaptationField())
            {
                TS_PacketAdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptationFieldControl());
            }
            printf("%010d ", TS_PacketId);
            TS_PacketHeader.Print();
            if (TS_PacketHeader.hasAdaptationField()) 
            { 
                TS_PacketAdaptationField.Print(); 
            }

            if(TS_PacketHeader.getPID() == 174){


                xPES_Assembler::eResult Result = PES_Assembler174.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);
                switch (Result)
                {
                case xPES_Assembler::eResult::StreamPackedLost: printf("PcktLost "); break;
                case xPES_Assembler::eResult::AssemblingStarted:
                    printf("Started ");
                    PES_Assembler174.PrintPESH();
                    break;
                case xPES_Assembler::eResult::AssemblingContinue:
                    printf("Continue ");
                    break;

                case xPES_Assembler::eResult::AssemblingFinished: { printf("Finished ");

                    printf("PES: Len=%d", PES_Assembler174.getNumPacketBytes());

                    fwrite(PES_Assembler174.getPacket(), 1, PES_Assembler174.getNumPacketBytes(), Video264);

                    PES_Assembler174.xBufferReset();
                }break;
                default: break;
                }
                printf("\n");
            
            }
            else if (TS_PacketHeader.getPID() == 136) {

                xPES_Assembler::eResult Result = PES_Assembler136.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);
                switch (Result)
                {
                case xPES_Assembler::eResult::StreamPackedLost: printf("PcktLost "); break;
                case xPES_Assembler::eResult::AssemblingStarted:
                    printf("Started ");
                    PES_Assembler136.PrintPESH();
                    break;
                case xPES_Assembler::eResult::AssemblingContinue:
                    printf("Continue ");
                    break;

                case xPES_Assembler::eResult::AssemblingFinished: { printf("Finished ");

                    printf("PES: Len=%d", PES_Assembler136.getNumPacketBytes());
                  
                    fwrite(PES_Assembler136.getPacket(), 1, PES_Assembler136.getNumPacketBytes(), AudioMP2);

                  
                    PES_Assembler136.xBufferReset();
                }break;
                default: break;
                }
                printf("\n");


            }
           
        }

        TS_PacketId++;

    }
    

    fclose(TransportStreamFile);
    fclose(AudioMP2);
    fclose(Video264);


    
}

