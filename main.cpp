#include "chip8.h"
#include "SDL_Layer.h"
#include <SDL.h>

#include <time.h>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>


int main(int argc, char** argv)
{
	//display buffer is 64x32, we need to scale it
	int resScale = std::stoi(argv[1]);
	//delay between cycles
	int cycleDelay = 4;
	const char* rom = argv[2];

	SDL_Layer interpreter("CHIP-8 Interpreter", VIDEO_WIDTH * resScale, VIDEO_HEIGHT * resScale, VIDEO_WIDTH, VIDEO_HEIGHT);

	//if unable to initialise SDL video/audio subsystem
	if (interpreter.flag = false)
	{
		return 1;
		//print error?
	}

	Chip8 chip8;
	chip8.LoadROM(rom);
	chip8.speed = cycleDelay;


	//SDL pitch param is the number of bytes in a row of pixel data
	int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

	auto lastCycleTime = std::chrono::system_clock::now();

	bool quit = false;
	float f = 1 / 60;

	while (!quit)
	{
		quit = interpreter.ProcessInput(chip8.keypad, chip8.pGameSpeed);

		cycleDelay = chip8.speed;

		//print speedToggle
		//std::cout << "Speed Toggle: " << chip8.speed << std::endl;

		////print cycleDelay
		//std::cout << "Cycle Delay: " << cycleDelay << std::endl;

		

		auto currentTime = std::chrono::system_clock::now();
		auto timeDiff = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		//check if enough time has passed between cycles
		if (timeDiff > cycleDelay)
		{
			lastCycleTime = currentTime;

			chip8.Cycle();

			interpreter.Update(chip8.video, videoPitch, VIDEO_WIDTH * resScale, VIDEO_HEIGHT * resScale);
		}
	}

	return 0;

	//auto t1 = std::chrono::high_resolution_clock::now();

	//obj.LoadROM("test.txt");

	//auto t2 = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	//std::cout << duration << std::endl;

	//std::cout << obj.randByte(obj.randGen) << std::endl;

	//return 0;
}
