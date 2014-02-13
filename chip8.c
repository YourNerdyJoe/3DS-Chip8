/**********************************
* CHIP 8 - Homebrew 3DS Rev 0.00.1*
*            S4rk                 *
***********************************/
#include "chip8.h"

void initEmu(){
    /* Emulator Initialization */
    int i = 0;


    DWORD fontSet[80] = {
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


    Chip8.PC = 0x200;
    Chip8.opcode = 0;
    Chip8.REG_I = 0;
    Chip8.stack_pointer = 0;
    Chip8.inGame = true;


    for(i = 0; i < SCREEN_SIZE; i++){
        Chip8.gameScreen[i] = 0;
    }

    for(i = 0; i < STACK_SIZE; i++){
        Chip8.stack[i] = 0;
        Chip8.reg[i] = 0;
    }

    for(i = 0; i < MEMORY_SIZE; i++){
        Chip8.memory[i] = 0;
    }

    for(i = 0; i < FONT_SET; i++){
        Chip8.memory[i] = fontSet[i];
    }


    initSDL();

}

void initSDL(){
    /* SDL TEST */

    if(SDL_Init(SDL_INIT_VIDEO) < 0 ){
        printf("SDL Init Error: %s", SDL_GetError());
        exit(1);
    }

    SDL_WM_SetCaption("Chip8 Emulator - SDL Test St4rk", "St4rk");

    Chip8.background = SDL_SetVideoMode(640, 480,32,SDL_SWSURFACE);

    if(Chip8.background== NULL){
        printf("SDL SetVideoMode Error: %s", SDL_GetError());
        exit(1);
    }
    loopEmu();

}

void loadROM(){
    /* Load Game ROM */
    int i;
    FILE *ROM;
    ROM = fopen("tetris.c8", "rb");

    if(ROM == NULL){
        printf("ROM not found !!\n");
        exit(1);
    }

    fseek(ROM , 0 , SEEK_END);
	Uint32 ROM_SIZE = ftell(ROM);

	rewind(ROM);

	char *ROM_BUFFER = (char*)malloc(sizeof(char) * ROM_SIZE);

	if(ROM_BUFFER == NULL){
        printf("Erro on memory buffer\n");
        exit(1);
	}


    fread(ROM_BUFFER, 1, ROM_SIZE, ROM);

    if((MEMORY_SIZE - 512) > ROM_SIZE){
        for(i = 0; i < ROM_SIZE; i++){
            Chip8.memory[512 + i] = ROM_BUFFER[i];
        }
    }else{
        printf("ROM SIZE ERROR\n");
    }

    free(ROM_BUFFER);
    fclose(ROM);
}


void loopEmu(){
    /* Emulator Loop */
    loadROM();
    while(Chip8.inGame == true){
        instruEmu();
        gameKey();
        drawGrf();
    }
}

void instruEmu(){
	    int i;
	    Chip8.opcode = Chip8.memory[Chip8.PC] << 8 | Chip8.memory[Chip8.PC + 1];
        printf("OPCODE: %X\n", Chip8.opcode);

		switch(Chip8.opcode & 0xF000)
		{
			case 0x0000:
				switch(Chip8.opcode & 0x000F)
				{
					case 0x0000: // 0x00E0: Clears the screen
						for(i = 0; i < SCREEN_SIZE; ++i){
                            Chip8.gameScreen[i] = 0x0;
						}

						Chip8.drawFlag = true;
						Chip8.PC += 2;

					break;

					case 0x000E: // 0x00EE: Returns from subroutine
						--Chip8.stack_pointer;			// 16 levels of Chip8.stack, decrease Chip8.stack pointer to prevent overwrite
						Chip8.PC = Chip8.stack[Chip8.stack_pointer];	// Put the stored return address from the Chip8.stack back into the program counter
						Chip8.PC += 2;		// Don't forget to increase the program counter!
					break;

					default:
						printf ("Unknown Chip8.opcode [0x0000]: 0x%X\n", Chip8.opcode);
				}
			break;

			case 0x1000: // 0x1NNN: Jumps to address NNN
				Chip8.PC = Chip8.opcode & 0x0FFF;
			break;

			case 0x2000: // 0x2NNN: Calls subroutine at NNN.
				Chip8.stack[Chip8.stack_pointer] = Chip8.PC;			// Store current address in Chip8.stack
				++Chip8.stack_pointer;					// Increment Chip8.stack pointer
				Chip8.PC = Chip8.opcode & 0x0FFF;	// Set the program counter to the address at NNN
			break;

		case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
				if(Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] == (Chip8.opcode & 0x00FF))
					Chip8.PC += 4;
				else
					Chip8.PC += 2;
			break;

			case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
				if(Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] != (Chip8.opcode & 0x00FF))
					Chip8.PC += 4;
				else
					Chip8.PC += 2;
			break;

			case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
				if(Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] == Chip8.reg[(Chip8.opcode & 0x00F0) >> 4])
					Chip8.PC += 4;
				else
					Chip8.PC += 2;
			break;

			case 0x6000: // 0x6XNN: Sets VX to NN.
				Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] = Chip8.opcode & 0x00FF;
				Chip8.PC += 2;
			break;

			case 0x7000: // 0x7XNN: Adds NN to VX.
				Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] += Chip8.opcode & 0x00FF;
				Chip8.PC += 2;
			break;

			case 0x8000:
				switch(Chip8.opcode & 0x000F)
				{
					case 0x0000: // 0x8XY0: Sets VX to the value of VY
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] = Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
						Chip8.PC += 2;
					break;

					case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] |= Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
						Chip8.PC += 2;
					break;

					case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] &= Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
						Chip8.PC += 2;
					break;

					case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] ^= Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
						Chip8.PC += 2;
					break;

					case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
						if(Chip8.reg[(Chip8.opcode & 0x00F0) >> 4] > (0xFF - Chip8.reg[(Chip8.opcode & 0x0F00) >> 8]))
							Chip8.reg[0xF] = 1; //carry
						else
							Chip8.reg[0xF] = 0;
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] += Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
						Chip8.PC += 2;
					break;

					case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
						if(Chip8.reg[(Chip8.opcode & 0x00F0) >> 4] > Chip8.reg[(Chip8.opcode & 0x0F00) >> 8])
							Chip8.reg[0xF] = 0; // there is a borrow
						else
							Chip8.reg[0xF] = 1;
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] -= Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
						Chip8.PC += 2;
					break;

					case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
						Chip8.reg[0xF] = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] & 0x1;
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] >>= 1;
						Chip8.PC += 2;
					break;

					case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
						if(Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] > Chip8.reg[(Chip8.opcode & 0x00F0) >> 4])	// VY-VX
							Chip8.reg[0xF] = 0; // there is a borrow
						else
							Chip8.reg[0xF] = 1;
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] = Chip8.reg[(Chip8.opcode & 0x00F0) >> 4] - Chip8.reg[(Chip8.opcode & 0x0F00) >> 8];
						Chip8.PC += 2;
					break;

					case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
						Chip8.reg[0xF] = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] >> 7;
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] <<= 1;
						Chip8.PC += 2;
					break;

					default:
						printf ("Unknown Chip8.opcode [0x8000]: 0x%X\n", Chip8.opcode);
				}
			break;

			case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
				if(Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] != Chip8.reg[(Chip8.opcode & 0x00F0) >> 4])
					Chip8.PC += 4;
				else
					Chip8.PC += 2;
			break;

			case 0xA000: // ANNN: Sets I to the address NNN
				Chip8.REG_I = Chip8.opcode & 0x0FFF;
				Chip8.PC += 2;
			break;

			case 0xB000: // BNNN: Jumps to the address NNN plus V0
				Chip8.PC = (Chip8.opcode & 0x0FFF) + Chip8.reg[0];
			break;

			case 0xC000: // CXNN: Sets VX to a random number and NN
				Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (Chip8.opcode & 0x00FF);
				Chip8.PC += 2;
			break;

			case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
						 // Each row of 8 pixels is read as bit-coded starting from memory location I;
						 // I value doesn't change after the execution of this instruction.
						 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
						 // and to 0 if that doesn't happen
			{
				DWORD x = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8];
				DWORD y = Chip8.reg[(Chip8.opcode & 0x00F0) >> 4];
				DWORD height = Chip8.opcode & 0x000F;
				DWORD pixel;
                int yline, xline;
				Chip8.reg[0xF] = 0;
				for (yline = 0; yline < height; yline++)
				{
					pixel = Chip8.memory[Chip8.REG_I + yline];
					for(xline = 0; xline < 8; xline++)
					{
						if((pixel & (0x80 >> xline)) != 0)
						{
							if(Chip8.gameScreen[(x + xline + ((y + yline) * 64))] == 1)
							{
								Chip8.reg[0xF] = 1;
							}
							Chip8.gameScreen[x + xline + ((y + yline) * 64)] ^= 1;
						}
					}
				}

				Chip8.drawFlag = true;
				Chip8.PC += 2;
			}
			break;

			case 0xE000:
				switch(Chip8.opcode & 0x00FF)
				{
					case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
						if(Chip8.gamePad[Chip8.reg[(Chip8.opcode & 0x0F00) >> 8]] != 0)
							Chip8.PC += 4;
						else
							Chip8.PC += 2;
					break;

					case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
						if(Chip8.gamePad[Chip8.reg[(Chip8.opcode & 0x0F00) >> 8]] == 0)
							Chip8.PC += 4;
						else
							Chip8.PC += 2;
					break;

					default:
						printf ("Unknown Chip8.opcode [0xE000]: 0x%X\n", Chip8.opcode);
				}
			break;

			case 0xF000:
				switch(Chip8.opcode & 0x00FF)
				{
					case 0x0007: // FX07: Sets VX to the value of the delay timer
						Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] = 0;
						Chip8.PC += 2;
					break;

					case 0x000A: // FX0A: A key press is awaited, and then stored in VX
					{
                        int i;
						bool keyPress = false;

						for(i = 0; i < 16; ++i)
						{
							if(Chip8.gamePad[i] != 0)
							{
								Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] = i;
								keyPress = true;
							}
						}

						// If we didn't received a keypress, skip this cycle and try again.
						if(!keyPress)
							return;

						Chip8.PC += 2;
					}

					break;

					case 0x0015: // FX15: Sets the delay timer to VX
						//delay_timer = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8];
						Chip8.PC += 2;
					break;

					case 0x0018: // FX18: Sets the sound timer to VX
						//sound_timer = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8];
						Chip8.PC += 2;
					break;

					case 0x001E: // FX1E: Adds VX to I
						if(Chip8.REG_I + Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
							Chip8.reg[0xF] = 1;
						else
							Chip8.reg[0xF] = 0;
						Chip8.REG_I += Chip8.reg[(Chip8.opcode & 0x0F00) >> 8];
						Chip8.PC += 2;
					break;

					case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
						Chip8.REG_I = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] * 0x5;
						Chip8.PC += 2;
					break;

					case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
						Chip8.memory[Chip8.REG_I]     = Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] / 100;
						Chip8.memory[Chip8.REG_I + 1] = (Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] / 10) % 10;
						Chip8.memory[Chip8.REG_I + 2] = (Chip8.reg[(Chip8.opcode & 0x0F00) >> 8] % 100) % 10;
						Chip8.PC += 2;
                    break;

				case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
					for (i = 0; i <= ((Chip8.opcode & 0x0F00) >> 8); ++i)
						Chip8.memory[Chip8.REG_I + i] = Chip8.reg[i];

					// On the original interpreter, when the operation is done, I = I + X + 1.
					Chip8.REG_I += ((Chip8.opcode & 0x0F00) >> 8) + 1;
					Chip8.PC += 2;
				break;

				case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I
					for (i = 0; i <= ((Chip8.opcode & 0x0F00) >> 8); ++i)
						Chip8.reg[i] = Chip8.memory[Chip8.REG_I + i];

					// On the original interpreter, when the operation is done, I = I + X + 1.
					Chip8.REG_I += ((Chip8.opcode & 0x0F00) >> 8) + 1;
					Chip8.PC += 2;
				break;

				default:
					printf ("Unknown opcode [0xF000]: 0x%X\n", Chip8.opcode);
			}
			break;

			default:
				printf ("Unknown Chip8.opcode: 0x%X\n", Chip8.opcode);
		}

}

void drawGrf(){
    /* Graphic Engine */
    SDL_FillRect(Chip8.background, NULL, 0x000000);
    if(Chip8.drawFlag == true){
        int x, y;       /* X, Y, W, H, */
        SDL_Rect pixel = {0, 0, 9,9};
        Uint8 ePixel = 0;
        for(x = 0; x < 64; x++){
            for(y = 0; y < 32; y++){
                ePixel = Chip8.gameScreen[x + y * 64];

                if(ePixel != 0){
                    pixel.x = x * 9;
                    pixel.y = y * 9;
                    SDL_FillRect(Chip8.background, &pixel, SDL_MapRGB(Chip8.background->format, 32,225,255));
                }
            }
        }
    }

    SDL_Flip(Chip8.background);
}

void gameKey(){
    SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                break;

                case SDL_KEYUP:
                    if(event.key.keysym.sym == SDLK_ESCAPE)

                    break;
                case SDL_QUIT:

                break;
            }
        }

}

