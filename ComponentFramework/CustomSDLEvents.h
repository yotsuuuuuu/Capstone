#pragma once
#include "SDL3/SDL.h"


namespace CustomEvent {

	inline Uint32 AUDIO_MENU_EVENT = 0;
	inline Uint32 SONG_SELECTED_EVENT = 0;
	inline Uint32 PLAYER_RESET_EVENT = 0;
	inline void Initilize() {
		AUDIO_MENU_EVENT = SDL_RegisterEvents(1);
		SONG_SELECTED_EVENT = SDL_RegisterEvents(2);
		PLAYER_RESET_EVENT = SDL_RegisterEvents(3);
	}
};