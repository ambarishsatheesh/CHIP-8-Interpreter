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
	float cycleDelay = 2;
	const char* rom = argv[2];

	SDL_Layer* interpreter = new SDL_Layer("CHIP-8 Interpreter", VIDEO_WIDTH * resScale, VIDEO_HEIGHT * resScale, VIDEO_WIDTH, VIDEO_HEIGHT);

	//if unable to initialise SDL video/audio subsystem
	if (interpreter->flag == false)
	{
		return 1;
		//print error?
	}

	Chip8* chip8 = new Chip8();
	chip8->LoadROM(rom);
	chip8->speed = cycleDelay;

	//SDL pitch param is the number of bytes in a row of pixel data
	int videoPitch = sizeof(chip8->video[0]) * VIDEO_WIDTH;

	auto lastCycleTime = std::chrono::system_clock::now();

	bool quit = false;

	while (!quit)
	{
		quit = interpreter->ProcessInput(chip8->keypad, chip8->pGameSpeed);

		cycleDelay = chip8->speed;

		//cycleDelay-independent filter refresh
		interpreter->Filter(chip8->video, videoPitch, VIDEO_WIDTH * resScale, VIDEO_HEIGHT * resScale);

		auto currentTime = std::chrono::system_clock::now();
		auto timeDiff = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		//check if enough time has passed between cycles
		if (timeDiff > cycleDelay)
		{
			lastCycleTime = currentTime;

			chip8->Cycle();

			interpreter->Update(chip8->video, videoPitch, VIDEO_WIDTH * resScale, VIDEO_HEIGHT * resScale);
		}
	}

	delete interpreter;
	delete chip8;

	return 0;
}
