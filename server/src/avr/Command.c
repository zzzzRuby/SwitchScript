#include <avr/interrupt.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <vm.h>

enum SerialCommand
{
    SerialCommand_Signal = 1,
    SerialCommand_Upload = 2,
    SerialCommand_HeartBeat = 3
};

typedef union
{
    struct {
        uint8_t command : 4;
        union {
            struct {
                uint8_t lastPart : 1;
                uint8_t reserved : 3;
            } upload;
        } desc;
    };
    uint8_t raw;
} SerialCommandData;

static RingBuffer_t g_CommandBuffer;
static uint8_t g_Buffer[64];
static bool g_Downloading = false;

ISR(USART1_RX_vect)
{
    RingBuffer_Insert(&g_CommandBuffer, UDR1);
}

void Command_Init(void) {
	Serial_Init(19200, false);
    RingBuffer_InitBuffer(&g_CommandBuffer, g_Buffer, sizeof(g_Buffer));
}

void Command_Update(void) {
    if (RingBuffer_IsEmpty(&g_CommandBuffer))
        return;

    SerialCommandData commandData;
    commandData.raw = RingBuffer_Peek(&g_CommandBuffer);

    switch(commandData.command) {
        case SerialCommand_Signal:
            RingBuffer_Remove(&g_CommandBuffer);
            VM_Signal();
            break;
        case SerialCommand_Upload: {
            if (RingBuffer_GetCount(&g_CommandBuffer) < 24)
                return;
            RingBuffer_Remove(&g_CommandBuffer);

            uint8_t blockSize = RingBuffer_Remove(&g_CommandBuffer);

            uint8_t low = RingBuffer_Remove(&g_CommandBuffer);
            uint8_t high = RingBuffer_Remove(&g_CommandBuffer);

            uint16_t offset = ((uint16_t)high) * 0x100 + low;
            
            if (!g_Downloading) {
                VM_Stop();
                g_Downloading = true;
            }

            uint8_t* buffer = VM_Heap();

            for(uint8_t i = 0;i < blockSize;i++) {
                buffer[i] = RingBuffer_Remove(&g_CommandBuffer);
            }

            VM_LoadProgram(buffer, blockSize, offset);

            if (commandData.desc.upload.lastPart) {
                g_Downloading = false;
            }
            
            break;
        }
        case SerialCommand_HeartBeat:
            RingBuffer_Remove(&g_CommandBuffer);
            break;
        default:
            RingBuffer_Remove(&g_CommandBuffer);
            break;
    }
}

bool Command_IsDownloading(void) { return g_Downloading; }