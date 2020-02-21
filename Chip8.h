#pragma once
#include "defines.h"

#include <random>

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_MAX = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONT_SIZE = 80;
const unsigned int FONT_START_ADDRESS = 0x50;

class Chip8
{
public:
	Chip8();
	void LoadROM(char const* filename);
	void Cycle();

	//public accessed by main.cpp
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};	//64 px * 32 px display memory buffer
	bool keypad[KEY_COUNT]{};
	//game speed control
	float speed = 0;
	float* pGameSpeed = &speed;

private:
	uint8_t registers[REGISTER_COUNT]{};	//dedicated CPU storage
	uint8_t memory[MEMORY_MAX]{};		//general memory
	uint16_t index{};	//Index Register - stores memory addresses for use in operations
	uint16_t pc{};	//Program Counter - holds address of next instruction
	uint16_t stack[STACK_LEVELS]{};	//keep track of execution order (call stack)
	uint8_t sp{};	//Stack Pointer (to index of stack array) - keep track of stack level where most recent value was placed
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint16_t opcode;

	std::vector<uint8_t> keyList;

	std::default_random_engine randGen;
	std::uniform_int_distribution<int> randByte; //uint8_t not valid template parameter?

	void Table0();
	void Table8();
	void TableE();
	void TableF();

	//dummmy void function in case invalid opcode is called
	//this will be default function called if proper func pointer is not set
	void OP_NULL();

//----------------------------------
//			Opcode declarations
//----------------------------------
												
	//CLS
	void OP_00E0();
	//RET
	void OP_00EE();
	//JP addr
	void OP_1nnn();
	//CALL addr
	void OP_2nnn();
	//SE Vx, byte
	void OP_3xkk();
	//SNE Vx, byte
	void OP_4xkk();
	//SE Vx, Vy
	void OP_5xy0();
	//LD Vx, byte
	void OP_6xkk();
	//ADD Vx, byte
	void OP_7xkk();
	//LD Vx, Vy
	void OP_8xy0();
	//OR Vx, Vy
	void OP_8xy1();
	//AND Vx, Vy
	void OP_8xy2();
	//XOR Vx, Vy
	void OP_8xy3();
	//ADD Vx, Vy
	void OP_8xy4();	
	//SUB Vx, Vy
	void OP_8xy5();	
	//SHR Vx {, Vy}
	void OP_8xy6();
	//Alternate SHR Vx { , Vy }
	void OP_8xy6_alt();
	//SUBN Vx, Vy
	void OP_8xy7();
	//SHL Vx {, Vy}
	void OP_8xyE();
	//Alternate SHL Vx {, Vy}
	void OP_8xyE_alt();
	//SNE Vx, Vy
	void OP_9xy0();
	//LD I, addr
	void OP_Annn();
	//JP V0, addr
	void OP_Bnnn();
	//RND Vx, byte
	void OP_Cxkk();
	//DRW Vx, Vy, nibble
	void OP_Dxyn();
	//SKP Vx
	void OP_Ex9E();
	//SKNP Vx
	void OP_ExA1();
	//LD Vx, DT
	void OP_Fx07();
	//LD Vx, K
	void OP_Fx0A();
	//LD DT, Vx
	void OP_Fx15();
	//LD ST, Vx
	void OP_Fx18();
	//ADD I, Vx
	void OP_Fx1E();
	//LD F, Vx
	void OP_Fx29();
	//LD B, Vx
	void OP_Fx33();
	//LD [I], Vx
	void OP_Fx55();
	//Alternate LD [I], Vx
	void OP_Fx55_alt();
	//LD Vx, [I]
	void OP_Fx65();
	//Alternate LD [I], Vx
	void OP_Fx65_alt();

	//define pointer-to-function type
	using Chip8Func = void (Chip8::*)();
	typedef void (Chip8::*Chip8Func)();
	//index up to 0xF + 1 (16)
	Chip8Func table[0xF + 1]{ &Chip8::OP_NULL };
	//index up to 0xE + 1 (15)
	Chip8Func table0[0xE + 1]{ &Chip8::OP_NULL };
	//index up to 0xE + 1 (15)
	Chip8Func table8[0xE + 1]{ &Chip8::OP_NULL };
	//index up to 0xE + 1 (15)
	Chip8Func tableE[0xE + 1]{ &Chip8::OP_NULL };
	//index up to 0x65 + 1 (102)
	Chip8Func tableF[0x65 + 1]{ &Chip8::OP_NULL };
};

