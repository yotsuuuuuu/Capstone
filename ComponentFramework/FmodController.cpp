#include "FmodController.h"
#include <fftw3.h>
#include <cmath>

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
	
	
	
	if (channel == nullptr || playing == false)
	{
		result = system->playSound(numOfsounds[songnum_], 0, false, &channel);
		channel->setVolume(volume / 100.0f);
	}
	else
	{
		/*channel->setPaused(true);
		system->playSound(numOfsounds[songnum_], 0, false, &channel);*/
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


void FmodController::Volume(float volume_)
{
	volume += volume_;

	if (volume > 100)
	{
		volume = 100;
	}
	else if (volume < 0)
	{
		volume = 0;
	}

	if (channel)
	{
		channel->setVolume(volume/100.0f);
	}
}

AudioBands FmodController::AnalyzeAudioOffline(int songnum_)
{
	//creates audio bands object
	AudioBands bands = { 0.0f,0.0f,0.0f };

	
	unsigned int lengthPCM = 0;//length cannot be negatice
	int channels = 0;
	int bits = 0;
	
	FMOD_SOUND_FORMAT format;
	FMOD_SOUND_TYPE type;

	//gets the format of te sound ex: pcm16 memeory format after fmod decodes it
	numOfsounds[songnum_]->getFormat(&type, &format, &channels, &bits);	
	//gets the length  of the sound in PCM
	numOfsounds[songnum_]->getLength(&lengthPCM, FMOD_TIMEUNIT_PCM);
	
	void* ptr1 = nullptr;
	void* ptr2 = nullptr;
	unsigned int len1 = 0, len2 = 0;

	// gets the size get ptr1 is where it starts in memory and len1 is the size of the sound in memory
	//	ptr2 is the second block of the memeory if the sond is long len2 is the size of the second block of memory
	numOfsounds[songnum_]->lock(0, lengthPCM * channels * (bits / 8), &ptr1, &ptr2, &len1, &len2); 
	//have to unlock it later
	short* samples = static_cast<short*>(ptr1);  // access PCM16 samples
	
	const int fftSize = 2048;       // Window size for FFT

	//flexible sample rate if the song is not 44100 it will still work
	float sampleRate;
	numOfsounds[songnum_]->getDefaults(&sampleRate, nullptr);
	


	//allocating the fftw buffers (OMG VULKAN BUFFER REFERENCE not really)
	double* in = (double*)fftw_malloc(sizeof(double) * fftSize);
	fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (fftSize / 2 + 1));

	//Creating the plan for the FFT, we can reuse this plan for multiple executions if the input size remains the same
	fftw_plan plan = fftw_plan_dft_r2c_1d(fftSize, in, out, FFTW_MEASURE);

	unsigned int windowCount = 0;
	double binWidth = static_cast<double>(sampleRate) / fftSize;
	//gets me pi according to the internet i can be wrong
	double pi = 3.14159265358979323846;

	for (unsigned int i = 0; i < lengthPCM; i += fftSize)
	{
		// Copy audio samples into the input buffer for FFTW 
		for (int j = 0; j < fftSize; j++)
		{
			double sample = 0.0;

			if (i + j < lengthPCM)//make sure that it gets even the last samples that are less than the fft size
			{
				sample = static_cast<double>(samples[i + j]) / 32768.0; // Normalize PCM16
			}
			// Hann window
			double hann = 0.5 * (1.0 - cos(2.0 * pi * j / (fftSize - 1)));
			in[j] = sample * hann;
		}

		// Execute the FFT does thge magic 
		fftw_execute(plan);


		// Analyze the FFT output to calculate energy in low, mid, and high frequency bands
		float lowEnergy = 0.0f;
		float midEnergy = 0.0f;
		float highEnergy = 0.0f;


		for (int k = 0; k < fftSize / 2 + 1; k++)
		{
			double real = out[k][0];
			double imag = out[k][1];

			// Power spectrum (energy)
			double power = real * real + imag * imag;
			//you canr ead the name but gets frequency
			double frequency = k * binWidth;

			if (frequency >= 20 && frequency < 250)
				lowEnergy += power;
			else if (frequency >= 250 && frequency < 4000)
				midEnergy += power;
			else if (frequency >= 4000 && frequency <= 20000)
				highEnergy += power;
		}
		//adding to global totals for each band
		bands.low += lowEnergy;
		bands.mid += midEnergy;
		bands.high += highEnergy;

		windowCount++;
	}

	//Average over all windows
	if (windowCount > 0)
	{
		bands.low /= windowCount;
		bands.mid /= windowCount;
		bands.high /= windowCount;
	}

	// NORMAILZE TIME 
	float total = bands.low + bands.mid + bands.high;
	if (total > 0.0f)
	{
		bands.low /= total;
		bands.mid /= total;
		bands.high /= total;
	}

	printf("Low: %f, Mid: %f, High: %f\n", bands.low, bands.mid, bands.high);
	
	//DO NOT USE DELETE USE FFTW_FREE TO FREE THE DATA
	fftw_destroy_plan(plan);
	fftw_free(in);
	fftw_free(out);

	numOfsounds[songnum_]->unlock(ptr1, ptr2, len1, len2); // Unlock the sound data after processing
	
	
	//incorparate more bands for noise map

	return bands;
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
