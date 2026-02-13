#include "FmodController.h"
#include <fftw3.h>

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
		//channel->setPaused(true);
		/*system->playSound(numOfsounds[songnum_], 0, false, &channel);*/
		channel->stop();
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

void FmodController::DummyFunction()
{
	const int fftSize = 2048;

	fftw_complex* in, * out;
	fftw_plan plan;

	in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftSize);
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftSize);
	plan = fftw_plan_dft_1d(fftSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	fftw_execute(plan); // Execute the FFT
	printf("\n data between herer \n");
	fftw_print_plan(plan);// Print the FFT plan (for debugging)
	printf("\n PRINTINF FROM DUMMY");

}

AudioBands FmodController::AnalyzeAudioOffline(int songnum_)
{
	//creates audio bands object
	AudioBands bands = { 0.0f,0.0f,0.0f };

	
	FMOD::Sound* sound = nullptr;

	unsigned int lengthPCM = 0;//length cannot be negatice
	int channels = 0;
	int bits = 0;
	
	FMOD_SOUND_FORMAT format;
	FMOD_SOUND_TYPE type;

	//gets the format of te sound ex: pcm16 memeory format after fmod decodes it
	numOfsounds[songnum_]->getFormat(&type, &format, &channels, &bits);	
	//gets the length  of the sound in PCM
	numOfsounds[songnum_]->getLength(&lengthPCM, FMOD_TIMEUNIT_PCM);


	//tells the memeory size (cool thing i wanna see)
	float audioMemory = lengthPCM * channels * bits / 8;
	
	void* ptr1 = nullptr;
	void* ptr2 = nullptr;
	unsigned int len1 = 0, len2 = 0;

	// gets the size get ptr1 is where it starts in memory and len1 is the size of the sound in memory
	//	ptr2 is the second block of the memeory if the sond is long len2 is the size of the second block of memory
	numOfsounds[songnum_]->lock(0, lengthPCM * channels * bits / 8, &ptr1, &ptr2, &len1, &len2); 
	//have to unlock it later
	short* samples = static_cast<short*>(ptr1);  // access PCM16 samples
	
	const int fftSize = 2048;       // Window size for FFT
	const float sampleRate = 44100; // Standard sample rate

	//DO NOT USE DELETE USE FFTW_FREE TO FREE THE DATA
	
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
