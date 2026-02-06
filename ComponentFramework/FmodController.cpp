#include "FmodController.h"
FmodController::FmodController(FMOD::System* system_)
{
	system = system_;
}

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

void FmodController::createSystem()
{
	result = FMOD::System_Create(&system);
	

	result = system->init(32, FMOD_INIT_NORMAL, nullptr);
	

	numOfsounds.resize(nameOfsounds.size());

	for (int i = 0; i < numOfsounds.size(); i++)
	{
		result = system->createSound(nameOfsounds[i], FMOD_DEFAULT, 0, &numOfsounds[i]);
	}
}

FmodController::~FmodController()
{
	for (int i = 0; i < numOfsounds.size(); i++)
	{
		numOfsounds[i]->release();
	}
	system->close();
	system->release();
}
