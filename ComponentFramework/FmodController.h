#pragma once
#include <fmod.hpp>
#include <vector>
#include "fmod_common.h"
class FmodController
{
private:
	FMOD::System* system;
	std::vector<FMOD::Sound*> numOfsounds;
	std::vector<const char*> nameOfsounds;
	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = FMOD_OK;
	void* extradriverdata = 0;

public:
	FmodController(FMOD::System* system_);
	void addSong(const char* wave_);
	void playsong(int songnum_);
	void createSystem();
	~FmodController();
};

