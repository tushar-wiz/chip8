#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define FONTSET_ADDRESS 0x50

uint16_t opcodes;
//program memory location begins at 512 (0x200)
//The uppermost 256 bytes (0xF00-0xFFF)(3840-4095) are reserved for display refresh
// 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
// 1 byte x 4K Memory
uint8_t memory[4096];

// 1 byte x 16 Registers
uint8_t V[16];

// 1 byte index register
uint16_t I;

// 2 byte program Counter
uint16_t pc;

// 1 byte x 16 gor keypad state
uint8_t keypad[16];

// 1 byte x 2K Video Memory
uint8_t gfx[64 * 32];

// stack and stack pointer (To make the emulator run not in actual chip 8)
uint16_t stack[16];
uint16_t sp;

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

// 1byte delay timer
static uint8_t delayTimer;

// 1 byte sound timer
static uint8_t soundTimer;

void initialize();
void emulateCycle();
void Dxyn();
void printScreen();

int main(){
    initialize();

    for(;;){
        emulateCycle();
        //system("clear");
        printScreen();
    }
    return 0;
}

void initialize(){
    pc = 0x200;
    opcodes = 0;
    I = 0;
    sp = 0;

    for(int i=0;i<16;i++)
        V[i] = 0;

    for(int i=0;i<4096;i++)
        memory[i] = 0;

    // Load fontset into memory locs
    for(int i=0;i<80;i++)
        memory[FONTSET_ADDRESS + i] = chip8_fontset[i];

    // Load Program into memory
    uint8_t* buf;
    char fileName[] = "test_opcode.ch8";
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

    uint8_t temp,val;
    // Decode opcode

    switch (opcodes & 0xF000){
    case 0x00E0:
        switch (opcodes & 0x000F){
        case 0x0:
            for(int i=0;i<(64*32);i++){
                gfx[i] = 0x00;
            }
            break;
        
        case 0xE:
            --sp;
	        pc = stack[sp];
            break;
        
        default:
            break;
        }
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
        if(V[(opcodes & 0x0F00)>>8] != V[(opcodes & 0x00F0)>>4])
            pc += 2;
            break;
    case 0xA000:
        I = (opcodes & 0x0FFF);
        break;
    case 0xB000:
        pc = (opcodes & 0x0FFF) + V[0x0];
        break;
    case 0xC000:
        V[(opcodes & 0x0F00)>>8] = (rand()%256) & (opcodes & 0x00FF);
        break;
    case 0xD000:
        // To be Implemented
        Dxyn();
        break;    
    case 0xE000:
        if((opcodes & 0x00FF) == 0x9E){
            if(keypad[V[(opcodes & 0x0F00)>>8]] == 0xFF)
                pc += 2;
        }
        else{
            if(keypad[V[(opcodes & 0x0F00)>>8]] == 0x00)
                pc += 2;
            }
        break;

    case 0xF000:
        switch (opcodes & 0x00FF){
        case 0x07:
            V[(opcodes & 0x0F00)>>8] = delayTimer;
            break;
        case 0x0A:
            temp = (opcodes & 0x0F00) >> 8;
            if(keypad[0]) V[temp] = 0;
            else if(keypad[1]) V[temp] = 1;
            else if(keypad[2]) V[temp] = 2;
            else if(keypad[3]) V[temp] = 3;
            else if(keypad[4]) V[temp] = 4;
            else if(keypad[5]) V[temp] = 5;
            else if(keypad[6]) V[temp] = 6;
            else if(keypad[7]) V[temp] = 7;
            else if(keypad[8]) V[temp] = 8;
            else if(keypad[9]) V[temp] = 9;
            else if(keypad[10]) V[temp] = 10;
            else if(keypad[11]) V[temp] = 11;
            else if(keypad[12]) V[temp] = 12;
            else if(keypad[13]) V[temp] = 13;
            else if(keypad[14]) V[temp] = 14;
            else if(keypad[15]) V[temp] = 15;
            else pc -= 2;
            break;
        
        case 0x15:
            delayTimer = V[(opcodes & 0x0F00) >> 8];
            break;
        
        case 0x18:
            soundTimer = V[(opcodes & 0x0F00) >> 8];
            break;
        
        case 0x1E:
            I += V[(opcodes & 0x0F00) >> 8];
            break;
        
        case 0x29:
            I = FONTSET_ADDRESS + (V[(opcodes & 0x0F00) >> 8] * 5);
            break;
        
        case 0x33:
            val = V[(opcodes & 0x0F00) >> 8];
            memory[I+2] = val%10;
            val /= 10;
            memory[I+1] = val%10;
            val /= 10;
            memory[I] = val%10;
            break;
        
        case 0x55:
            for(uint8_t i=0;i<V[(opcodes & 0x0F00) >> 8];i++)
                memory[I + i] = V[i];
            break;
        
        case 0x65:
            for(uint8_t i=0;i<V[(opcodes & 0x0F00) >> 8];i++)
                V[i] = memory[I + i];
            break;            
        default:
            break;
        }
    default:
        break;
    }
    pc += 2;
    // Timers
}


// Copied Function from internet
void Dxyn()
{
	uint8_t Vx = (opcodes & 0x0F00u) >> 8u;
	uint8_t Vy = (opcodes & 0x00F0u) >> 4u;
	uint8_t height = opcodes & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = V[Vx] % 64;
	uint8_t yPos = V[Vy] % 32;

	V[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[I + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint8_t* screenPixel = &gfx[(yPos + row) * 64 + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					V[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void printScreen(){
    for(int j=0;j<32;j++){
        for(int i=0;i<64;i++){

            if(gfx[(64*j)+i] == 0xFF){
               printf("???");
            }
            else{
                printf(" ");
            }
            //printf("%d",gfx[(64*j)+i]);
        }
        printf("\n");
    }

}