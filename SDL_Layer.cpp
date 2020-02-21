#include "SDL_Layer.h"
#include <iostream>


//https://wiki.libsdl.org/APIByCategory

SDL_Layer::SDL_Layer(const char* title, int winWidth, int winHeight, int textureWidth, int textureHeight)
{
	//initialise video subsystem with error logging
	if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialise SDL: %s", SDL_GetError());
		//set flag for error catching
		flag = false;
	}

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winWidth, winHeight, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
}

SDL_Layer::~SDL_Layer()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

SDL_Rect SDL_Layer::Lines(int topLeftX, int topLeftY, int rectWidth, int rectHeight)
{
	SDL_Rect rect;
	rect.x = topLeftX;
	rect.y = topLeftY;
	rect.w = rectWidth;
	rect.h = rectHeight;

	return rect;
}

void SDL_Layer::Filter(const void* buffer, int pitch, int winWidth, int winHeight)
{
	switch (filterNum)
	{
	case 0:
	{
		SDL_RenderPresent(renderer);
	}break;
	case 1:
	{
		//Scanline
		for (int topLeftY = 0; topLeftY < winHeight; topLeftY += 4)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &Lines(0, topLeftY, winWidth, 2));
		}
		SDL_RenderPresent(renderer);
	}break;
	}
}

void SDL_Layer::Update(const void* buffer, int pitch, int winWidth, int winHeight)
{
	//nullptr param updates entire texture
	SDL_UpdateTexture(texture, nullptr, buffer, pitch);
	//set texture colour
	SDL_SetTextureColorMod(texture, red, green, blue);
	//copy texture to rendering target - source and dest nullptr for entire texture
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	//sdl rendering operates on backbuffer; you need to present the backbuffer to screen
	SDL_RenderPresent(renderer);
}

bool SDL_Layer::ProcessInput(bool* keys, float* pGameSpeed)
{
	bool quit = false;

	//event structure to store event information
	SDL_Event event;

	//process each event in turn
	while (SDL_PollEvent(&event))
	{
		//handle each event type separately
		switch (event.type)
		{
		case SDL_QUIT:
		{
			quit = true;
		}
		break;

		case SDL_KEYDOWN:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
			{
				quit = true;
			}break;

			case SDLK_x:
			{
				keys[0] = 1;
			} break;

			case SDLK_1:
			{
				keys[1] = 1;
			} break;

			case SDLK_2:
			{
				keys[2] = 1;
			} break;

			case SDLK_3:
			{
				keys[3] = 1;
			} break;

			case SDLK_q:
			{
				keys[4] = 1;
			} break;

			case SDLK_w:
			{
				keys[5] = 1;
			} break;

			case SDLK_e:
			{
				keys[6] = 1;
			} break;

			case SDLK_a:
			{
				keys[7] = 1;
			} break;

			case SDLK_s:
			{
				keys[8] = 1;
			} break;

			case SDLK_d:
			{
				keys[9] = 1;
			} break;

			case SDLK_z:
			{
				keys[0xA] = 1;
			} break;

			case SDLK_c:
			{
				keys[0xB] = 1;
			} break;

			case SDLK_4:
			{
				keys[0xC] = 1;
			} break;

			case SDLK_r:
			{
				keys[0xD] = 1;
			} break;

			case SDLK_f:
			{
				keys[0xE] = 1;
			} break;

			case SDLK_v:
			{
				keys[0xF] = 1;
			} break;

			//slow down
			case SDLK_EQUALS:
			{
				if (*pGameSpeed == 0.25)
				{
					*pGameSpeed = 0;
				}
				else
				{
					*pGameSpeed /= 2;
				}
			} break;

			//fast forward
			case SDLK_MINUS:
			{
				if (*pGameSpeed == 0)
				{
					*pGameSpeed = 0.25;
				}
				else
				{
					*pGameSpeed *= 2;
				}
				//set minimum speed
				*pGameSpeed = *pGameSpeed > 32 ? 32 : *pGameSpeed;
			} break;
			case SDLK_TAB:
			{
				if (filterNum == 1)
				{
					filterNum = 0;
				}
				else
				{
					filterNum = 1;
				}
			}break;
			case SDLK_CAPSLOCK:
			{
				if (colourNum == 4)
				{
					colourNum = 0;
				}
				else
				{
					colourNum += 1;
				}
				switch (colourNum)
				{
				case 0:
				{
					red = 255;
					blue = 255;
					green = 255;
				}break;
				case 1:
				{
					red = 0;
					green = 0;
					blue = 255;
				}break;
				case 2:
				{
					red = 0;
					green = 255;
					blue = 0;
				}break;
				case 3:
				{
					red = 255;
					green = 0;
					blue = 0;
				}break;
				case 4:
				{
					red = 255;
					green = 255;
					blue = 0;
				}break;
				}
			}
			}
		} break;

		case SDL_KEYUP:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_x:
			{
				keys[0] = 0;
			} break;

			case SDLK_1:
			{
				keys[1] = 0;
			} break;

			case SDLK_2:
			{
				keys[2] = 0;
			} break;

			case SDLK_3:
			{
				keys[3] = 0;
			} break;

			case SDLK_q:
			{
				keys[4] = 0;
			} break;

			case SDLK_w:
			{
				keys[5] = 0;
			} break;

			case SDLK_e:
			{
				keys[6] = 0;
			} break;

			case SDLK_a:
			{
				keys[7] = 0;
			} break;

			case SDLK_s:
			{
				keys[8] = 0;
			} break;

			case SDLK_d:
			{
				keys[9] = 0;
			} break;

			case SDLK_z:
			{
				keys[0xA] = 0;
			} break;

			case SDLK_c:
			{
				keys[0xB] = 0;
			} break;

			case SDLK_4:
			{
				keys[0xC] = 0;
			} break;

			case SDLK_r:
			{
				keys[0xD] = 0;
			} break;

			case SDLK_f:
			{
				keys[0xE] = 0;
			} break;

			case SDLK_v:
			{
				keys[0xF] = 0;
			} break;
			}
		} break;

		}
	}
	return quit;
}