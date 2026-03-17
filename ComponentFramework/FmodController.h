#pragma once
#include <fmod.hpp>
#include <vector>
#include "CoreStructs.h"


class FmodController
{
private:
	FMOD::System* system;
	std::vector<FMOD::Sound*> sounds;
	std::vector<std::string> nameOfsounds;
	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = FMOD_OK;
	void* extradriverdata = 0;
	float volume = 25.0f;

public:
	FmodController(){};
	void addSong(const char* wave_);
	void addSong(const std::vector<const char*>& wave_);
	std::string getSongName(int songnum_);
	bool AddSonginFile();
	void InitilizeSongs();
	void playsong(int songnum_);
	void playsong(AudioState state_);
	bool createSystem();
	void Volume(float volume_);


	std::vector<AudioBands> AnalyzeAudioOffline(int songunum_);
	std::vector<AudioBands> AnalyzeAudioOnline();
	//system that gets the fmod sound data that fram puts it inot bands then sends it to the vulkan rednerer for the shader system. NEEDS TO GET SIZE
	~FmodController();
};

