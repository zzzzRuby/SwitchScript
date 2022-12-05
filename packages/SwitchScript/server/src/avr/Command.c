#include <avr/interrupt.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <vm.h>
#include "../Command.h"

enum SerialCommand
{
    SerialCommand_Signal = 1,
    SerialCommand_Upload = 2,
    SerialCommand_UploadEnd = 3,
    SerialCommand_HeartBeat = 4
};

enum SerialResult
{
    SerialResult_UploadResult = 1,
    SerialResult_HeartBeat = 2
};

static RingBuffer_t g_CommandBuffer;
static uint8_t g_Buffer[64];
static bool g_Downloading = false;

ISR(USART1_RX_vect)
{
    RingBuffer_Insert(&g_CommandBuffer, UDR1);
}

void Command_Init(void) {
	Serial_Init(9600, false);
    RingBuffer_InitBuffer(&g_CommandBuffer, g_Buffer, sizeof(g_Buffer));
}

void Command_Update(void) {
    while(true) {
        if (RingBuffer_IsEmpty(&g_CommandBuffer))
            return;

        uint8_t command = RingBuffer_Peek(&g_CommandBuffer);

        switch(command) {
            case SerialCommand_Signal: {
                if (RingBuffer_GetCount(&g_CommandBuffer) < 3)
                    return;
                RingBuffer_Remove(&g_CommandBuffer);
                uint8_t low = RingBuffer_Remove(&g_CommandBuffer);
                uint8_t high = RingBuffer_Remove(&g_CommandBuffer);

                int16_t value = high << 8 | low;

                VM_Signal(value);
                break;
            }
            case SerialCommand_Upload: {
                if (RingBuffer_GetCount(&g_CommandBuffer) < 24)
                    return;
                RingBuffer_Remove(&g_CommandBuffer);

                uint8_t blockSize = RingBuffer_Remove(&g_CommandBuffer);

                uint8_t low = RingBuffer_Remove(&g_CommandBuffer);
                uint8_t high = RingBuffer_Remove(&g_CommandBuffer);

                uint16_t offset = high << 8 | low;
                
                if (!g_Downloading) {
                    if (!VM_IsTerminated())
                        VM_Stop();
                    g_Downloading = true;
                    VM_StartLoadProgram();
                }

                uint8_t* buffer = VM_Heap();

                for(uint8_t i = 0;i < 20;i++) {
                    buffer[i] = RingBuffer_Remove(&g_CommandBuffer);
                }

                VM_LoadProgram(buffer, blockSize, offset);

                break;
            }
            case SerialCommand_UploadEnd:
                RingBuffer_Remove(&g_CommandBuffer);
                VM_EndLoadProgram();
                g_Downloading = false;
            case SerialCommand_HeartBeat:
                RingBuffer_Remove(&g_CommandBuffer);
                break;
            default:
                RingBuffer_Remove(&g_CommandBuffer);
                break;
        }
    }
}

bool Command_IsDownloading(void) { return g_Downloading; }