#pragma once
#include <fmod.hpp>
#include <vector>


struct AudioBands
{
	float sub; //20 Hz-60 Hz
	float bass; //60 Hz-130 Hz
	float highBass; //130 Hz-262 Hz
	float lowMid; //250 Hz-523 Hz
	float midMid; //523 Hz-1046 Hz
	float highMid; //1 kHz-2093 Hz
	float lowHigh; //2 kHz-41896 Hz
	float midHigh; //4 kHz-8 kHz
	float highHigh; //8 kHz-12 kHz
	float air; //12 kHz-20 kHz

	
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
	float volume = 100.0f;

public:
	FmodController(){};
	void addSong(const char* wave_);
	void playsong(int songnum_);
	bool createSystem();
	void Volume(float volume_);

	std::vector<AudioBands> AnalyzeAudioOffline(int songunum_);
	//system that gets the fmod sound data that fram puts it inot bands then sends it to the vulkan rednerer for the shader system. NEEDS TO GET SIZE
	~FmodController();
};

