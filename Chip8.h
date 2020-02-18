#pragma once
#include "defines.h"

#include <cstdint>
#include <chrono>
#include <random>

//https://austinmorlan.com/posts/chip8_emulator/#conclusion

class Chip8
{
public:
	uint8_t registers[16]{};	//dedicated CPU storage
	uint8_t memory[4096]{};		//general memory
	uint16_t index{};	//Index Register - stores memory addresses for use in operations
	uint16_t pc{};	//Program Counter - holds address of next instruction
	uint16_t stack[16]{};	//keep track of execution order (call stack)
	uint8_t sp{};	//Stack Pointer (to index of stack array) - keep track of stack level where most recent value was placed
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint8_t keypad[16]{};
	uint32_t video[2048]{};	//64 px * 32 px display memory buffer
	uint16_t opcode;


	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte; //uint8_t not valid template parameter?


	Chip8();
	void LoadROM(char const* filename);

private:

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
	//alternate SHR Vx { , Vy }
	void OP_8xy6_alt();
	//SUBN Vx, Vy
	void OP_8xy7();
	//SHL Vx {, Vy}
	void OP_8xyE();
	//alternate SHL Vx {, Vy}
	void OP_8xyE_alt();
	//SNE Vx, Vy
	void OP_9xy0();
	//LD I, addr
	void OP_Annn();
	//JP V0, addr
	void OP_Bnnn();
	//RND Vx, byte
	void OP_Cxkk();

	//8xy6, 8xye, Fx55, and Fx65 - don't use cowgod as reference
};

