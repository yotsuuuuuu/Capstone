#pragma once
#include "SDL3/SDL.h"


namespace CustomEvent {

	inline Uint32 AUDIO_MENU_EVENT = 0;
	
	inline void Initilize() {
		AUDIO_MENU_EVENT = SDL_RegisterEvents(1);
	}
};