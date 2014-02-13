/**********************************
* CHIP 8 - Homebrew 3DS Rev 0.00.1*
*            S4rk                 *
***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#define SCREEN_SIZE 2048
#define REG_T 16
#define STACK_SIZE 16
#define GAME_PAD 16
#define MEMORY_SIZE 4096
#define FONT_SET 80

typedef unsigned char BYTE;
typedef unsigned short DWORD;
typedef char bool;
typedef enum{true, false};
typedef struct _Chip8 _Chip8;

struct _Chip8{
    DWORD opcode;
    BYTE reg[REG_T];
    BYTE memory[MEMORY_SIZE];

    DWORD REG_I;
    DWORD PC;

    DWORD stack[STACK_SIZE];
    DWORD stack_pointer;

    bool drawFlag;
    bool inGame;

    BYTE gamePad[GAME_PAD];
    BYTE gameScreen[SCREEN_SIZE];

    SDL_Surface* background;
};

_Chip8 Chip8;


void initEmu();
void initSDL(); /* TESTE */
void instruEmu();
void loopEmu();
void loadROM();
void drawGrf();
void gameKey();

