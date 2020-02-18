/*
future changes/additions:
-smart pointers
-better error handling
-implement toggle between both opcode variations for 8xy6, 8xye, Fx55, and Fx65

*/

#include "Chip8.h"
#include <fstream>


const unsigned int START_ADDRESS = 0x200;
const unsigned int FONT_SIZE = 80;
const unsigned int FONT_START_ADDRESS = 0x50;

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
	: randGen(std::chrono::system_clock::now().time_since_epoch().count()) //member initialisation list with system clock seed
{
	//initialise program counter
	pc = START_ADDRESS;

	//load fonts into memory starting at 0x50
	for (unsigned int i = 0; i < FONT_SIZE; ++i)
	{
		memory[FONT_START_ADDRESS + i] = fontset[i];
	}

	//initialise RNG - can use randByte(randGen) to get random number between 0 and 255
	randByte = std::uniform_int_distribution<uint8_t>(0, 255);
}

void Chip8::LoadROM(const char* filename)
{
	//Open file as binary stream and set initial pos to end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		//get current stream pos and set that as size; allocate buffer to hold contents
		std::streampos size = file.tellg();

		auto* memoryStart = &memory[START_ADDRESS];

		file.seekg(0, std::ios::beg); //set pos to beg of stream
		file.read(reinterpret_cast<char*>(memoryStart), size); //extract size characters from stream and store in memory
		file.close();
	}
	else
	{
		//error handling here
	}
}

//----------------------------------
//			Opcode definitions
//----------------------------------

//Clear display
void Chip8::OP_00E0()
{
	//set all bytes in display buffer to 0
	memset(video, 0, sizeof(video));
}

//Return from subroutine
void Chip8::OP_00EE()
{
	--sp;
	pc = stack[sp];
}

//Jump to location nnn
void Chip8::OP_1nnn()
{
	//set pc to address of opcode
	pc = NNN(opcode);
}

//Call subroutine at nnn
void Chip8::OP_2nnn()
{
	//put current pc on top of stack
	stack[sp] = pc;
	++sp;
	//set pc to address of opcode
	pc = NNN(opcode);
}

//Skip next instruction if Vx = kk
void Chip8::OP_3xkk()
{
	if (registers[X(opcode)] == KK(opcode))
	{
		pc += 2;
	}
}

//Skip next instruction if Vx != kk
void Chip8::OP_4xkk()
{
	if (registers[X(opcode)] != KK(opcode))
	{
		pc += 2;
	}
}

//Skip next instruction if Vx == Vy
void Chip8::OP_5xy0()
{
	if (registers[X(opcode)] == Y(opcode))
	{
		pc += 2;
	}
}

//Set Vx = kk
void Chip8::OP_6xkk()
{
	registers[X(opcode)] = KK(opcode);
}

//Set Vx = Vx + kk
void Chip8::OP_7xkk()
{
	registers[X(opcode)] += KK(opcode);
}

//Set Vx = Vy
void Chip8::OP_8xy0()
{
	registers[X(opcode)] = registers[Y(opcode)];
}

//Set Vx = Vx OR Vy
void Chip8::OP_8xy1()
{
	registers[X(opcode)] |= registers[Y(opcode)];
}

//Set Vx = Vx AND Vy
void Chip8::OP_8xy2()
{
	registers[X(opcode)] &= registers[Y(opcode)];
}

//Set Vx = Vx XOR Vy
void Chip8::OP_8xy3()
{
	registers[X(opcode)] ^= registers[Y(opcode)];
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
}

//Set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::OP_8xy5()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = registers[Vx] > registers[Vy] ? 1 : 0;

	registers[Vx] -= registers[Vy];
}

//Set Vx = Vx SHR 1
void Chip8::OP_8xy6()
{
	uint8_t Vx = X(opcode);

	//Save least significant bit in VF (bitwise & binary 1)
	registers[0xF] = registers[Vx] & 0x1u;
	registers[Vx] >>= 1;
}

//alternate version (correct?) 
//Set Vx = Vy SHR 1
//Store the value of register VY shifted right one bit in register VX
//Set register VF to the least significant bit prior to the shift
void Chip8::OP_8xy6_alt()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = registers[Vy] & 0x1u;
	registers[Vx] = registers[Vy] >> 1;
}

//Set Vx = Vy - Vx, set VF = NOT borrow
void Chip8::OP_8xy7()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = registers[Vy] > registers[Vx] ? 1 : 0;

	registers[Vx] = registers[Vy] - registers[Vx];
}

//Set Vx = Vx SHL 1
void Chip8::OP_8xyE()
{
	uint8_t Vx = X(opcode);

	//Save most significant bit in VF (bitwise & binary 10000000 then >>)
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
	registers[Vx] <<= 1;
}

//alternate version (correct?) 
//Set Vx = Vx SHL 1
//Store the value of register VY shifted left one bit in register VX
//Set register VF to the most significant bit prior to the shift
void Chip8::OP_8xyE_alt()
{
	uint8_t Vx = X(opcode);
	uint8_t Vy = Y(opcode);

	registers[0xF] = (registers[Vy] & 0x80u) >> 7u;
	registers[Vx] <= registers[Vy] << 1;
}

//Skip next instruction if Vx != Vy
void Chip8::OP_9xy0()
{
	if (registers[X(opcode)] != registers[Y(opcode)])
	{
		pc += 2;
	}
}

//Set I = nnn (I = index register)
void Chip8::OP_Annn()
{
	index = NNN(opcode);
}

//Jump to address nnn + V0
void Chip8::OP_Bnnn()
{
	pc = NNN(opcode) + registers[0];
}

//Set Vx = random byte AND kk
void Chip8::OP_Cxkk()
{
	registers[X(opcode)] = randByte(randGen) & KK(opcode);
}