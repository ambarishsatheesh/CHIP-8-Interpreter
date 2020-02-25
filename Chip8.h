/*
future changes/additions:
-smart pointers
-better error handling
-implement toggle between both opcode variations for 8xy6, 8xye, Fx55, and Fx65
-change hexadecimal to binary literals (C++14)
-implement sound output
-destructors?
-SDL_VideoQuit/ clean up SDL subsystems upon quit (atexit)
-GUI
*/

#include "Chip8.h"
#include <fstream>
#include <iostream>


//16 sprites representing characters
uint8_t fontset[FONT_SIZE] =
{
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


Chip8::Chip8()
	: randGen(CLOCKCOUNT) //randGen member initialisation list with system clock seed
{
	//initialise program counter
	pc = START_ADDRESS;

	//load fonts into memory starting at 0x50
	for (unsigned int i = 0; i < FONT_SIZE; ++i)
	{
		memory[FONT_START_ADDRESS + i] = fontset[i];
	}

	//put keys in list to avoid switch statements (OP_Fx0A)
	for (int i = 0; i < 16; ++i)
	{
		keyList.push_back(0);
	}

	//initialise RNG - can use randByte(randGen) to get random number between 0 and 255
	randByte = std::uniform_int_distribution<int>(0, 255);

	//function pointer table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;

}

void Chip8::LoadROM(const char* filename)
{
	//Open file as binary stream and set initial pos to end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		//get current stream pos and set that as size; allocate buffer to hold contents
		std::streampos size = file.tellg();

		auto memoryStart = &memory[START_ADDRESS];

		file.seekg(0, std::ios::beg); //set pos to beg of stream
		file.read(reinterpret_cast<char*>(memoryStart), size); //extract size characters from stream and store in memory
		file.close();
	}
	else
	{
		//error handling here
	}
}

//Intruction cycle (fetch-decode-execute)
void Chip8::Cycle()
{
	//opcode is 2 bytes but memory value is 1 byte
	//so we need to get memory[pc], turn it to 16-bit and combine with memory[pc+1]
	//e.g. 1010000 << 8 | 10011000 = 1101000010011000
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	//increment PC before execution
	pc += 2;

	//get first single digit (e.g. 0xd6ed will become d)
	//look up in fuction pointer table and execute
	(this->*(table[I(opcode)]))();

	//decrement delay timer if loaded with value
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	//decrement sound timer if loaded with value
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}

//function pointer calls
void Chip8::Table0()
{
	//function call is determined by dereferenced value in
	//the table at index provided by opcode bitwise calculation
	//e.g. tableF[0x65] = &Chip8::OP_Fx65;
	(this->*(table0[opcode & 0x00Fu]))();
}

void Chip8::Table8()
{
	(this->*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
	(this->*(tableE[opcode & 0x000Fu]))();

}

void Chip8::TableF()
{
	(this->*(tableF[KK(opcode)]))();
}

void Chip8::OP_NULL()
{

}

//----------------------------------
//			Opcode definitions
//----------------------------------

//Clear display
void Chip8::OP_00E0()
{
	//set all bytes in display buffer to 0
	memset(video, 0, sizeof(video));
	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Return from subroutine
void Chip8::OP_00EE()
{
	--sp;
	pc = stack[sp];
	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Jump to location nnn
void Chip8::OP_1nnn()
{
	//set pc to address of opcode
	pc = NNN(opcode);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Call subroutine at nnn
void Chip8::OP_2nnn()
{
	//put current pc on top of stack
	stack[sp] = pc;
	++sp;
	//set pc to address of opcode
	pc = NNN(opcode);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Skip next instruction if Vx = kk
void Chip8::OP_3xkk()
{
	if (registers[X(opcode)] == KK(opcode))
	{
		pc += 2;
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Skip next instruction if Vx != kk
void Chip8::OP_4xkk()
{
	if (registers[X(opcode)] != KK(opcode))
	{
		pc += 2;
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Skip next instruction if Vx == Vy
void Chip8::OP_5xy0()
{
	if (registers[X(opcode)] == registers[Y(opcode)])
	{
		pc += 2;
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = kk
void Chip8::OP_6xkk()
{
	registers[X(opcode)] = KK(opcode);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx + kk
void Chip8::OP_7xkk()
{
	registers[X(opcode)] += KK(opcode);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vy
void Chip8::OP_8xy0()
{
	registers[X(opcode)] = registers[Y(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx OR Vy
void Chip8::OP_8xy1()
{
	registers[X(opcode)] |= registers[Y(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx AND Vy
void Chip8::OP_8xy2()
{
	registers[X(opcode)] &= registers[Y(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx XOR Vy
void Chip8::OP_8xy3()
{
	registers[X(opcode)] ^= registers[Y(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx + Vy, set VF = carry
void Chip8::OP_8xy4()
{
	//used multiple times so store result
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	uint16_t sum = registers[Vx] + registers[Vy];

	//set overflow flag VF depending on sum
	//0xF (decimal 15) represents the VF register (16th/final register)
	registers[0xF] = sum > 255u ? 1 : 0;

	//Only the lowest 8 bits of the result are stored in Vx (0xFF is decimal 255)
	registers[Vx] = sum & 0xFFu;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::OP_8xy5()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = registers[Vx] > registers[Vy] ? 1 : 0;

	registers[Vx] -= registers[Vy];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx SHR 1
void Chip8::OP_8xy6()
{
	uint8_t Vx = X(opcode);

	//Save least significant bit in VF (bitwise & binary 1)
	registers[0xF] = registers[Vx] & 0x1u;
	registers[Vx] >>= 1;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Alternate version (correct?)
//Set Vx = Vy SHR 1
//Store the value of register VY shifted right one bit in register VX
//Set register VF to the least significant bit prior to the shift
void Chip8::OP_8xy6_alt()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = registers[Vy] & 0x1u;
	registers[Vx] = registers[Vy] >> 1;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vy - Vx, set VF = NOT borrow
void Chip8::OP_8xy7()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = registers[Vy] > registers[Vx] ? 1 : 0;

	registers[Vx] = registers[Vy] - registers[Vx];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = Vx SHL 1
void Chip8::OP_8xyE()
{
	uint8_t Vx = X(opcode);

	//Save most significant bit in VF (bitwise & binary 10000000 then >>)
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
	registers[Vx] <<= 1;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Alternate version (correct?)
//Set Vx = Vx SHL 1
//Store the value of register VY shifted left one bit in register VX
//Set register VF to the most significant bit prior to the shift
void Chip8::OP_8xyE_alt()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = (registers[Vy] & 0x80u) >> 7u;
	registers[Vx] = registers[Vy] << 1;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Skip next instruction if Vx != Vy
void Chip8::OP_9xy0()
{
	if (registers[X(opcode)] != registers[Y(opcode)])
	{
		pc += 2;
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set I = nnn (I = index register)
void Chip8::OP_Annn()
{
	index = NNN(opcode);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Jump to address nnn + V0
void Chip8::OP_Bnnn()
{
	pc = NNN(opcode) + registers[0];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = random byte AND kk
void Chip8::OP_Cxkk()
{
	registers[X(opcode)] = randByte(randGen) & KK(opcode);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Display n-byte sprite starting at memory address I at (Vx, Vy), set VF = collision
void Chip8::OP_Dxyn()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);
	//get only n bit from opcode
	uint8_t height = opcode & 0x000Fu;

	//Wrap if outside display co-ords
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		//start at memory address I
		uint8_t spriteByte = memory[index + row];

		//sprite will always be 8 pixels wide
		for (unsigned int col = 0; col < 8; ++col)
		{
			//0x80u >> col scans through byte, 1 bit at a time
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			if (spritePixel)
			{
				//collision with screen pixel
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				//same as XOR with sprite pixel (because sprite pixel is 1)
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Skip next instruction if key with value of Vx is pressed
void Chip8::OP_Ex9E()
{
	if (keypad[registers[X(opcode)]])
	{
		pc += 2;
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Skip next instruction if key with the value of Vx is not pressed
void Chip8::OP_ExA1()
{
	if (!keypad[registers[X(opcode)]])
	{
		pc += 2;
	}

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set Vx = delay timer value
void Chip8::OP_Fx07()
{
	registers[X(opcode)] = delayTimer;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Wait for a key press, store the value of the key in Vx 
void Chip8::OP_Fx0A()
{
	bool pressed = false;
	for (int i = 0; i < 16; ++i)
	{
		if (keyList[i])
		{
			registers[X(opcode)] = i;
			break;
			pressed = true;
		}
	}
	//if no key press, decrement PC by 2, causing instruction to repeat indefinitely
	pc -= !pressed ? 2 : 0;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set delay timer = Vx
void Chip8::OP_Fx15()
{
	delayTimer = registers[X(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set sound timer = Vx
void Chip8::OP_Fx18()
{
	soundTimer = registers[X(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;

}

//Set I = I + Vx
void Chip8::OP_Fx1E()
{
	index += registers[X(opcode)];

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Set I = address of sprite for digit Vx
void Chip8::OP_Fx29()
{
	//sprites are 5 bytes each, add multiple to 0x50 (start address)
	index = FONT_START_ADDRESS + (5 * registers[X(opcode)]);

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Store BCD representation of Vx in memory locations I, I+1 and I+2
//take the decimal value of Vx, and place the hundreds digit in memory at location in I,
//the tens digit at location I+1, and the ones digit at location I+2.
void Chip8::OP_Fx33()
{
	uint8_t decimalVal = registers[X(opcode)];

	memory[index + 2] = decimalVal % 10;
	decimalVal /= 10;
	memory[index + 1] = (decimalVal % 10);
	decimalVal /= 10;
	memory[index] = decimalVal % 10;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Store the values of registers V0 to VX inclusive in memory starting at address I
void Chip8::OP_Fx55()
{
	uint8_t Vx = X(opcode);
	for (int i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];

		//print current function
		std::cout << __FUNCTION__ << std::endl;
	}
}

//Alternate version (correct?)
//Store the values of registers V0 to VX inclusive in memory starting at address I
//I is set to I + X + 1 after operation
void Chip8::OP_Fx55_alt()
{
	uint8_t Vx = X(opcode);
	for (int i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
	index = index + Vx + 1;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}

//Fill registers V0 to VX inclusive with the values stored in memory starting at address I
void Chip8::OP_Fx65()
{
	uint8_t Vx = X(opcode);
	for (int i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];

		//print current function
		std::cout << __FUNCTION__ << std::endl;
	}
}

//Alternate version (correct?)
//Fill registers V0 to VX inclusive with the values stored in memory starting at address I
//I is set to I + X + 1 after operation
void Chip8::OP_Fx65_alt()
{
	uint8_t Vx = X(opcode);
	for (int i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}

	index = index + Vx + 1;

	//print current function
	std::cout << __FUNCTION__ << std::endl;
}
