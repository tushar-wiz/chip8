#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint16_t opcodes;
//program memory location begins at 512 (0x200)
//The uppermost 256 bytes (0xF00-0xFFF)(3840-4095) are reserved for display refresh
// 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
// 1 byte x 4K Memory
static uint8_t memory[4096];

// 1 byte x 16 Registers
static uint8_t V[16];

// 1 byte index register
static uint16_t I;

// 2 byte program Counter
static uint16_t pc;

// 1 byte x 16 gor keypad state
static uint8_t keypad[16];

// 1 byte x 2K Video Memory
static uint8_t gfx[64 * 32];

// stack and stack pointer (To make the emulator run not in actual chip 8)
static uint16_t stack[16];
static uint16_t sp;

static uint8_t chip8_fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize();
void emulateCycle();

int main(){
    initialize();
    return 0;
}

void initialize(){
    pc = 0x200;
    opcodes = 0;
    I = 0;
    sp = 0;

    for(int i=0;i<16;i++)
        V[i] = 0;

    // Load fontset into memory locs
    for(int i=0;i<80;i++)
        memory[i+80] = chip8_fontset[i];

    // Load Program into memory
    uint8_t* buf;
    char fileName[] = "Pong.ch8";
    FILE *ptr;
    ptr = fopen(fileName,"rb");

    fseek(ptr,0,SEEK_END);
    int n = ftell(ptr);
    fseek(ptr,0,SEEK_SET);
    buf = (uint8_t *)malloc(n+1);

    fread(buf,1,n,ptr);
    fclose(ptr);

    for(int i=0;i<n;i++){
        memory[i+512] = *(buf + i);
        //printf("%02x\n",memory[i+512]);
    }

    free(buf);
    buf = NULL;
}

void emulateCycle(){
    // Fetch opcode
    opcodes = (memory[pc] << 8)|(memory[pc+1]);

    // Decode opcode

    switch (opcodes & 0xF000){
    case 0x0000:
        break;
    
    case 0x1000:
        pc = (opcodes & 0x0FFF);
        break;
    
    case 0x2000:
        stack[sp] = pc;
        sp++;
        pc = opcodes & 0x0FFF;
        break;
    
    case 0x3000:
        if(V[(opcodes & 0x0F00)>>8] == (opcodes & 0x00FF))
            pc += 2;
        break;
    
    case 0x4000:
        if(V[(opcodes & 0x0F00)>>8] != (opcodes & 0x00FF))
            pc += 2;
        break;
    
    case 0x5000:
        if(V[(opcodes & 0x0F00)>>8] == V[(opcodes & 0x00F0)>>4])
            pc += 2;
        break;
    
    case 0x6000:
        V[(opcodes & 0x0F00)>>8] = (opcodes & 0x00FF);
        break;

    case 0x7000:
        V[(opcodes & 0x0F00)>>8] += (opcodes & 0x00FF);
        break;
    
    case 0x8000:
        switch(opcodes & 0x000F){
            case 0x0000:
                V[(opcodes & 0x0F00)>>8] = V[(opcodes & 0x00F0)>>4];
                break;
            
            case 0x0001:
                V[(opcodes & 0x0F00)>>8] |= V[(opcodes & 0x00F0)>>4];
                break;
            
            case 0x0002:
                V[(opcodes & 0x0F00)>>8] &= V[(opcodes & 0x00F0)>>4];
                break;
            
            case 0x0003:
                V[(opcodes & 0x0F00)>>8] ^= V[(opcodes & 0x00F0)>>4];
                break;
            
            case 0x0004:
                if(V[(opcodes & 0x00F0)>>4] > (0xFF - V[(opcodes & 0x0F00)>>8]))
                    V[0xF] = 1;
                else
                    V[0xF] = 0;
                V[(opcodes & 0x0F00)>>8] += V[(opcodes & 0x00F0)>>4];
                break;
            
            case 0x0005:
                if(V[(opcodes & 0x0F00)>>8] > V[(opcodes & 0x00F0)>>4])
                    V[0xF] = 1;
                else
                    V[0xF] = 0;
                V[(opcodes & 0x0F00)>>8] -= V[(opcodes & 0x00F0)>>4];
                break;
            
            case 0x0006:
                if(V[(opcodes & 0x0F00)>>8] & 0b1)
                    V[0xF] = 1;
                else
                    V[0xF] = 0;
                V[(opcodes & 0x0F00)>>8] >>= 1;
                break;
            
            case 0x0007:
                if(V[(opcodes & 0x00F0)>>4] > V[(opcodes & 0x0F00)>>8])
                    V[0xF] = 1;
                else
                    V[0xF] = 0;
                V[(opcodes & 0x0F00)>>8] = V[(opcodes & 0x00F0)>>4] - V[(opcodes & 0x0F00)>>8];
                break;
            
            case 0x000E:
                if((V[(opcodes & 0x0F00)>>8] & (0b1<<8))>>8)
                    V[0xF] = 1;
                else
                    V[0xF] = 0;
                V[(opcodes & 0x0F00)>>8] <<= 1;
                break;
        }
        case 0x9000:

    default:
        break;
    }

    // Timers
}