# Chip8 Interpreter
Chip 8 is a not an actual chip or a piece of hardware thus calling it an emulator is wrong. It is coded as an emulator, and generally beginners like me make this so we can move on to make more advanced such as the Gameboy and NES.  
* Chip8 has 36 different instructions
* 4KB memory
* 16 general purpose 8bit registers
* 16 keys for user control
* 64x32 monochrome display


## Building
To build this you must have the **SDL2** library installed and the **SDL2.dll** in the *root* folder  
Compiler Flags
```
g++ -o main.exe chip8.cpp -lmingw32 -lSDL2main -lSDL2 -std=c++14
```
To play you must have the chip8 ROM for a particular game and have it placed in the *root* folder  
Then add the filename of the ROM on line 378  
```
378| char fileName[] = "tetris.rom";
```

## Screenshots
I Tested some of the available ROMS from the internet  
[Pong](https://github.com/kripod/chip8-roms/blob/master/games/Pong%20(1%20player).ch8)
![Pong](/images/pong.png)  

[Tetris](https://github.com/badlogic/chip8/blob/master/roms/tetris.rom)
![Tetris](/images/tetris.png)  

[Test Opcode](https://github.com/corax89/chip8-test-rom)
![Test Opcode](/images/test.png)  

[Flight Runner](https://johnearnest.github.io/chip8Archive/play.html?p=flightrunner)
![Flight Runner](/images/flight_runner.png)

### Issues
Sometimes the compiled binary fails to start, just re running it works (Potential issue with SDL code)

### Old Folder
I started with coding in C but realised using Classes for this task would be better and shifted to C++
