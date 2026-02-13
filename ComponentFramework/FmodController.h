#pragma once
#include <fmod.hpp>
#include <vector>


struct AudioBands
{
	float low; //20 Hz-250 Hz
	float mid; //250 Hz-4 kHz
	float high; //4kHz-20 kHz
};


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
	FmodController(){};
	void addSong(const char* wave_);
	void playsong(int songnum_);
	bool createSystem();
	void DummyFunction();

	AudioBands AnalyzeAudioOffline(int songunum_);
	~FmodController();
};

