#pragma once
#include <fmod.hpp>
#include <vector>
#include "CoreStructs.h"


class FmodController
{
private:
	FMOD::System* system;
	std::vector<FMOD::Sound*> numOfsounds;
	std::vector<const char*> nameOfsounds;
	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = FMOD_OK;
	void* extradriverdata = 0;
	float volume = 0.0f;

public:
	FmodController(){};
	void addSong(const char* wave_);
	void playsong(int songnum_);
	void playsong(AudioState state_);
	bool createSystem();
	void Volume(float volume_);


	std::vector<AudioBands> AnalyzeAudioOffline(int songunum_);
	//system that gets the fmod sound data that fram puts it inot bands then sends it to the vulkan rednerer for the shader system. NEEDS TO GET SIZE
	~FmodController();
};

