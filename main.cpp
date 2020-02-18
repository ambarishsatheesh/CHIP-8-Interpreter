#include "chip8.h"
#include <time.h>
#include <chrono>
#include <iostream>

int main()
{
	Chip8 obj;

	auto t1 = std::chrono::high_resolution_clock::now();

	obj.LoadROM("test.txt");

	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	std::cout << duration << std::endl;

	std::cout << obj.randByte(obj.randGen) << std::endl;

	return 0;
}