#include <iostream>
#include <cstdint>
#include <cstring>

using namespace std;

#define startLocation 0x200
#define fontSetStart 0x50
#define screen_width 64
#define screen_height 32

uint8_t chip8_fontset[80] = {
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

class chip8{
public:

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

    // 1 byte x 2K Video Memory (64 x 32)
    uint32_t gfx[screen_width * screen_height];

    // stack and stack pointer (To make the emulator run not in actual chip 8)
    uint16_t stack[16];
    uint16_t sp;

    uint16_t opcode;

    uint8_t delayTimer;
    uint8_t soundTimer;

    // Class Defined Outside
    void loadProgram(); // Loads File into Memory
    
    chip8(){
        pc = startLocation;

        for(int i=0;i<80;i++)
            memory[i + fontSetStart] = chip8_fontset[i]; 
    }

    void op_00E0(){
        memset(gfx,0,sizeof(gfx));
    }

    void op_00EE(){
        --sp;
	    pc = stack[sp];
    }    
    
    void op_1(){
        pc = (opcode & 0x0FFF);
    }
    
    void op_2(){
        stack[sp] = pc;
        sp++;
        pc = opcode & 0x0FFF;
    }

    void op_3(){
        if(V[(opcode & 0x0F00)>>8] == (opcode & 0x00FF))
            pc += 2;
    }
    
    void op_4(){
        if(V[(opcode & 0x0F00)>>8] != (opcode & 0x00FF))
            pc += 2;
    }
    
    void op_5(){
        if(V[(opcode & 0x0F00)>>8] == V[(opcode & 0x00F0)>>4])
            pc += 2;
    }

    void op_6(){
        V[(opcode & 0x0F00)>>8] = (opcode & 0x00FF);
    }

    void op_7(){
        V[(opcode & 0x0F00)>>8] += (opcode & 0x00FF);
    }

    void op_8xy0(){
        V[(opcode & 0x0F00)>>8] = V[(opcode & 0x00F0)>>4];
    }

    void op_8xy1(){
        V[(opcode & 0x0F00)>>8] |= V[(opcode & 0x00F0)>>4];
    }
    
    void op_8xy2(){
        V[(opcode & 0x0F00)>>8] &= V[(opcode & 0x00F0)>>4];
    }

    void op_8xy3(){
        V[(opcode & 0x0F00)>>8] ^= V[(opcode & 0x00F0)>>4];
    }

    void op_8xy4(){
        if(V[(opcode & 0x00F0)>>4] > (0xFF - V[(opcode & 0x0F00)>>8]))
            V[0xF] = 1;
        else
            V[0xF] = 0;
        V[(opcode & 0x0F00)>>8] += V[(opcode & 0x00F0)>>4];
    }

    void op_8xy5(){
        if(V[(opcode & 0x0F00)>>8] > V[(opcode & 0x00F0)>>4])
            V[0xF] = 1;
        else
            V[0xF] = 0;
        V[(opcode & 0x0F00)>>8] -= V[(opcode & 0x00F0)>>4];
    }

    void op_8xy6(){
        V[0xF] = V[(opcode & 0x0F00)>>8] & 0x1;
        V[(opcode & 0x0F00)>>8] >>= 1;
    }

    void op_8xy7(){
        if(V[(opcode & 0x00F0)>>4] > V[(opcode & 0x0F00)>>8])
            V[0xF] = 1;
        else
            V[0xF] = 0;
        V[(opcode & 0x0F00)>>8] = V[(opcode & 0x00F0)>>4] - V[(opcode & 0x0F00)>>8];
    }

    void op_8xyE(){
        if((V[(opcode & 0x0F00)>>8] & (0b1<<8))>>8)
            V[0xF] = 1;
        else
            V[0xF] = 0;
        V[(opcode & 0x0F00)>>8] <<= 1;
    }

    void op_9(){
        if(V[(opcode & 0x0F00)>>8] != V[(opcode & 0x00F0)>>4])
            pc += 2;
    }

    void op_A(){
        I = (opcode & 0x0FFF);
    }

    void op_B(){
        pc = (opcode & 0x0FFF) + V[0];
    }

    void op_C(){
        V[(opcode & 0x0F00)>>8] = (rand()%256) & (opcode & 0x00FF);
    }

    void op_D(){
        uint8_t height = opcode & 0x000Fu;

        // Wrap if going beyond screen boundaries
        uint8_t xPos = V[(opcode & 0x0F00u) >> 8u] % screen_width;
        uint8_t yPos = V[(opcode & 0x00F0u) >> 4u] % screen_height;

        V[0xF] = 0;

        for (unsigned int row = 0; row < height; ++row){
            uint8_t spriteByte = memory[I + row];

            for (unsigned int col = 0; col < 8; ++col){
                uint8_t spritePixel = spriteByte & (0x80u >> col);
                uint32_t* screenPixel = &gfx[(yPos + row) * screen_width + (xPos + col)];

                // Sprite pixel is on
                if (spritePixel){
                    // Screen pixel also on - collision
                    if (*screenPixel == 0xFFFFFFFF){
                        V[0xF] = 1;
                    }
                    // Effectively XOR with the sprite pixel
                    *screenPixel ^= 0xFFFFFFFF;
                }
            }
        }    
    }    

    void op_Ex9E(){
        if(keypad[V[(opcode & 0x0F00)>>8]])
            pc += 2;
    }

    void op_ExA1(){
        if(!keypad[V[(opcode & 0x0F00)>>8]])
            pc += 2;
    }

    void op_Fx07(){
        V[(opcode & 0x0F00)>>8] = delayTimer;
    }

    void op_Fx0A(){
        uint8_t Vx = (opcode & 0x0F00) >> 8;
        if(keypad[0]) V[Vx] = 0;
        else if(keypad[1]) V[Vx] = 1;
        else if(keypad[2]) V[Vx] = 2;
        else if(keypad[3]) V[Vx] = 3;
        else if(keypad[4]) V[Vx] = 4;
        else if(keypad[5]) V[Vx] = 5;
        else if(keypad[6]) V[Vx] = 6;
        else if(keypad[7]) V[Vx] = 7;
        else if(keypad[8]) V[Vx] = 8;
        else if(keypad[9]) V[Vx] = 9;
        else if(keypad[10]) V[Vx] = 10;
        else if(keypad[11]) V[Vx] = 11;
        else if(keypad[12]) V[Vx] = 12;
        else if(keypad[13]) V[Vx] = 13;
        else if(keypad[14]) V[Vx] = 14;
        else if(keypad[15]) V[Vx] = 15;
        else pc -= 2;
    }

    void op_Fx15(){
        delayTimer = V[(opcode & 0x0F00) >> 8];
    }

    void op_Fx18(){
        soundTimer = V[(opcode & 0x0F00) >> 8];
    }

    void op_Fx1E(){
        I += V[(opcode & 0x0F00) >> 8];
    }

    void op_Fx29(){
        I = fontSetStart + (V[(opcode & 0x0F00) >> 8] * 5);
    }
    
    void op_Fx33(){
        uint8_t val = V[(opcode & 0x0F00) >> 8];
        memory[I+2] = val%10;
        val /= 10;
        memory[I+1] = val%10;
        val /= 10;
        memory[I] = val%10;
    }

    void op_Fx55(){
        for(uint8_t i=0;i<=V[(opcode & 0x0F00) >> 8];++i)
            memory[I + i] = V[i];
    }

    void op_Fx65(){
        for(uint8_t i=0;i <= V[(opcode & 0x0F00) >> 8];++i)
            V[i] = memory[I + i];
    }
};

void chip8::loadProgram(){
    uint8_t* buf;
    char fileName[] = "test_opcode.ch8";
    FILE *ptr;
    //opens the file for reading
    ptr = fopen(fileName,"rb");

    //takes the pointer to then end
    fseek(ptr,0,SEEK_END);
    // gets the total space required in bytes
    int n = ftell(ptr);
    //sets pointer back to start
    fseek(ptr,0,SEEK_SET);

    // creates buffer to store file
    buf = (uint8_t *)malloc(n+1);
    fread(buf,1,n,ptr);
    fclose(ptr);

    // takes buffer and stores it in the memory of the chip
    for(int i=0;i<n;i++){
        memory[i+startLocation] = *(buf + i);
        //printf("%02x\n",memory[i+512]);
    }

    delete[] buf;
    buf = NULL;
}



int main(){
    int n;
    chip8 c;
    c.loadProgram();
    return 0;
}