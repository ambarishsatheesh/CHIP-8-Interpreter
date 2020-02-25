#pragma once
#include <SDL.h>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class SDL_Layer
{
public:
	SDL_Layer(const char* title, int winWidth, int winHeight, int textureWidth, int textureHeight);
	~SDL_Layer();
	void Update(const void* buffer, int pitch, int winWidth, int winHeight);
	SDL_Rect Lines(int topLeftX, int topLeftY, int rectWidth, int rectHeight);
	void Filter(const void* buffer, int pitch, int winWidth, int winHeight);
	bool ProcessInput(bool* keys, float* pGameSpeed);

	uint8_t red{ 255 };
	uint8_t green{ 255 };
	uint8_t blue{ 255 };

	int filterNum = 0;
	int colourNum = 0;
	bool flag = true;

private:
	SDL_Window * window{};
	SDL_Renderer* renderer{};
	SDL_Texture* texture{};
};
