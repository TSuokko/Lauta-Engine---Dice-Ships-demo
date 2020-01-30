#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#define GLOBAL_SCALE 1
#define SCREEN_WIDTH 1080 * GLOBAL_SCALE
#define SCREEN_HEIGHT 1920 * GLOBAL_SCALE



class Globals {

public:

	static SDL_Event Global_Event;

	static SDL_Renderer *Global_Renderer;

	static SDL_Window *Global_Window;

	static SDL_Rect Camera;

};





