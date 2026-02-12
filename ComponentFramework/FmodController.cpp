#include "FmodController.h"

void FmodController::addSong(const char* wave_)
{
	nameOfsounds.push_back(wave_);
}

void FmodController::playsong(int songnum_)
{
	bool playing = false;
	bool stopped = false;
	if (channel)
	{
		channel->isPlaying(&playing);
	}
	if (channel == nullptr)
	{
		result = system->playSound(numOfsounds[songnum_], 0, false, &channel);
	}
	else
	{
		channel->setPaused(true);
		system->playSound(numOfsounds[songnum_], 0, false, &channel);
		channel->setPaused(false);
	}
	result = system->update();
}

bool FmodController::createSystem()
{
	result = FMOD::System_Create(&system);
	
	if (result != FMOD_OK)
		return false;

	result = system->init(512, FMOD_INIT_NORMAL, nullptr);
	
	if (result != FMOD_OK)
		return false;

	numOfsounds.resize(nameOfsounds.size());

	for (size_t i = 0; i < numOfsounds.size(); i++)
	{
		result = system->createSound(nameOfsounds[i], FMOD_DEFAULT, 0, &numOfsounds[i]);
	}
	return true;
}

AudioBands FmodController::AnalyzeAudioOffline(const char* path)
{
	//creates audio bands object
	AudioBands bands = { 0.0f,0.0f,0.0f };

	
	FMOD::Sound* sound = nullptr;

	if(system->createSound(path, FMOD_DEFAULT, 0, &sound) != FMOD_OK)
	return AudioBands();
}

FmodController::~FmodController()
{
	for (size_t i = 0; i < numOfsounds.size(); i++)
	{
		numOfsounds[i]->release();
	}
	system->close();
	system->release();
}
