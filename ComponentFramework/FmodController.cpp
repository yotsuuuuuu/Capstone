#include "FmodController.h"
#include <fftw3.h>
#include <cmath>
#include <filesystem>

void FmodController::addSong(const char* wave_)
{
	nameOfsounds.push_back(wave_);
}
void FmodController::addSong(const std::vector<const char*>& wave_)
{
	for (size_t i = 0; i < wave_.size(); i++)
	{
		nameOfsounds.push_back(wave_[i]);
	}
}
std::string FmodController::getSongName(int songnum_)
{
	if (songnum_ >= 0 && songnum_ < nameOfsounds.size())
	{

		//std::cout << name << std::endl;
		std::string filename = nameOfsounds[songnum_];

		size_t pos = filename.find_last_of('.');
		if (pos != std::string::npos)
		{
			filename = filename.substr(0, pos);

		}

		pos = filename.find_last_of('/');
		if (pos != std::string::npos)
		{
			filename = filename.substr(pos + 1);

		}

		for (int i = 0; i < filename.size(); i++)
		{
			if (filename[i] == '_')
			{
				filename[i] = ' ';
			}
			else if (filename[i] == '-')
			{
				filename[i] = ' ';
			}
			else if (filename[i] == '\\')
			{
				filename[i] = ' ';
			}
			else if (filename[i] == '.')
			{
				filename[i] = ' ';

			}
		}

		while (true)
		{
			size_t pos = filename.find("  ");
			if (pos == std::string::npos)
				break;
			filename.replace(pos, 2, " ");
		}



			return filename;
	}
	else
	{
		return "";
	}
	
}
bool FmodController::AddSonginFile()
{

	if (nameOfsounds.size() > 0)
	{
		nameOfsounds.clear();

	}

	std::string path = "./audio/";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_regular_file())
		{
			std::cout << "Not a regular file: " << entry.path() << std::endl;
			continue;
		}

		std::string name = entry.path().string();
		if (std::find(nameOfsounds.begin(), nameOfsounds.end(), name) == nameOfsounds.end())
		{
			nameOfsounds.push_back(name);
		}
		else
		{
			std::cout << "File already exists in the list: " << name << std::endl;
		}

	}
	return true;
}

void FmodController::playsong(int songnum_)
{
	FMOD::Sound* currentSound = nullptr;
	channel->getCurrentSound(&currentSound);

	bool playing = false;
	bool stopped = false;
	
	
	channel->isPlaying(&playing);

	if (playing)
	{
		channel->stop();
	}


	sounds->release();
	sounds = nullptr;
	
	result = system->createStream(nameOfsounds[songnum_].c_str(), FMOD_DEFAULT, 0, &sounds);
	
	if (result != FMOD_OK)
	{
		std::cout << "FMOD error loading "
			<< nameOfsounds[songnum_]
			<< ": "
				<< std::endl;
	}

	result = system->playSound(sounds, 0, false, &channel);
	channel->setVolume(volume / 100.0f);
	
	FMOD::Sound* sound = nullptr;
	channel->getCurrentSound(&sound);
	char name[256];

	sound->getName(name, 256);

	std::cout << "Currently playing: " << name << std::endl;

	result = system->update();
}
void FmodController::playsong(AudioState state_)
{
	switch (state_)
	{
	case AudioState::PLAY:
		if (channel)
			channel->setPaused(false);
		break;
	case AudioState::PAUSE:
		if (channel)
			channel->setPaused(true);
		break;
	case AudioState::STOP:
		if (channel)
			channel->stop();
		break;
	default:
		break;
	}
}

bool FmodController::createSystem()
{
	result = FMOD::System_Create(&system);
	
	if (result != FMOD_OK)
		return false;

	result = system->init(512, FMOD_INIT_NORMAL, nullptr);
	
	//dsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 1024);//sets window size can playa round with this 1024 is average so the audio wqont be to "noisy" but will be clean and update enough to give data fast
	//dsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, FMOD_DSP_FFT_WINDOW_HANNING);
	//channel->addDSP(0, dsp); // Add the DSP to the channel at index 0 (before the sound is processed)
	//dsp->setActive(true);
	system->getMasterChannelGroup(&masterGroup);

	system->createDSPByType(FMOD_DSP_TYPE_FFT, &dsp);
	dsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 1024);
	dsp->setParameterInt(FMOD_DSP_FFT_WINDOW, FMOD_DSP_FFT_WINDOW_HANNING);
	dsp->setActive(true);

	// Add DSP to master group before any sounds are played
	masterGroup->addDSP(0, dsp);

	if (result != FMOD_OK)
		return false;

	return true;
}


void FmodController::Volume(float volume_)
{
	volume = volume_;

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

SongTime FmodController::getTimeOfSong(int index_)
{
	unsigned int length;

	sounds->getLength(&length, FMOD_TIMEUNIT_MS);

	SongTime total;

	total.min = length / 60000;
	total.sec = (length % 60000) / 1000;

	return total;
}

SongTime FmodController::getCurrentTime()
{
	unsigned int positionInSong;

	channel->getPosition(&positionInSong, FMOD_TIMEUNIT_MS);
	
	SongTime currentPos;

	currentPos.min = positionInSong / 60000;
	currentPos.sec = (positionInSong % 60000) / 1000;

	return currentPos;
}



std::vector<AudioBands> FmodController::AnalyzeAudioOffline(int songnum_)
{
	//creates audio bands object
	std::vector<AudioBands> bandHolder;

	FMOD::Sound* tempsound = nullptr;

	system->createSound(nameOfsounds[songnum_].c_str(), FMOD_DEFAULT, 0, &tempsound);
	
	unsigned int lengthPCM = 0;//length cannot be negatice
	int channels = 0;
	int bits = 0;
	
	FMOD_SOUND_FORMAT format;
	FMOD_SOUND_TYPE type;

	//gets the format of te sound ex: pcm16 memeory format after fmod decodes it
	tempsound->getFormat(&type, &format, &channels, &bits);	
	//gets the length  of the sound in PCM
	tempsound->getLength(&lengthPCM, FMOD_TIMEUNIT_PCM);
	//
	//system->mixerSuspend(); // Suspend the mixer to safely access sound data

	void* ptr1 = nullptr;
	void* ptr2 = nullptr;
	unsigned int len1 = 0, len2 = 0;

	//flexible sample rate if the song is not 44100 it will still work
	float sampleRate;
	tempsound->getDefaults(&sampleRate, nullptr);


	// gets the size get ptr1 is where it starts in memory and len1 is the size of the sound in memory
	//	ptr2 is the second block of the memeory if the sond is long len2 is the size of the second block of memory
	tempsound->lock(0, lengthPCM * channels * (bits / 8), &ptr1, &ptr2, &len1, &len2); 
	//have to unlock it later
	short* samples = static_cast<short*>(ptr1);  // access PCM16 samples
	
	const int fftSize = 2048;       // Window size for FFT
	//allocating the fftw buffers (OMG VULKAN BUFFER REFERENCE not really)
	double* in = (double*)fftw_malloc(sizeof(double) * fftSize);
	fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (fftSize / 2 + 1));

	//Creating the plan for the FFT, we can reuse this plan for multiple executions if the input size remains the same
	fftw_plan plan = fftw_plan_dft_r2c_1d(fftSize, in, out, FFTW_ESTIMATE);

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
				int index = (i + j);
				sample = static_cast<double>(samples[index]) / 32768.0; // Normalize PCM16
			}
			// Hann window
			double hann = 0.5 * (1.0 - cos(2.0 * pi * j / (fftSize - 1)));
			in[j] = sample * hann;
		}

		// Execute the FFT does thge magic 
		fftw_execute(plan);


		// Analyze the FFT output to calculate energy in low, mid, and high frequency bands
		float subEnergy = 0.0f;
		float bassEnergy = 0.0f;
		float highBassEnergy = 0.0f;
		float lowMidEnergy = 0.0f;
		float midMidEnergy = 0.0f;
		float highMidEnergy = 0.0f;
		float lowHighEnergy = 0.0f;
		float midHighEnergy = 0.0f;
		float highHighEnergy = 0.0f;	
		float airEnergy = 0.0f;


		for (int k = 0; k < fftSize / 2 + 1; k++)
		{
			double real = out[k][0];
			double imag = out[k][1];

			// Power spectrum (energy)
			double power = real * real + imag * imag;
			//you canr ead the name but gets frequency
			double frequency = k * binWidth;

			if (frequency < 60)
				subEnergy += power;
			else if (frequency < 130)
				bassEnergy += power;
			else if (frequency <= 262)
				highBassEnergy += power;
			else if (frequency < 523)
				lowMidEnergy += power;
			else if (frequency < 1046)
				midMidEnergy += power;
			else if (frequency < 2093)
				highMidEnergy += power;
			else if (frequency < 4186)
				lowHighEnergy += power;
			else if (frequency < 8000)
				midHighEnergy += power;
			else if (frequency < 12000)
				highHighEnergy += power;
			else if (frequency <= 20000)
				airEnergy += power;
			
		}

		float totalEnergy = subEnergy + bassEnergy + highBassEnergy + lowMidEnergy + midMidEnergy + highMidEnergy + lowHighEnergy + midHighEnergy + highHighEnergy + airEnergy;
		if(totalEnergy == 0)
			totalEnergy = 1.0f; // Avoid division by zero

		bandHolder.push_back({ subEnergy/totalEnergy, bassEnergy/totalEnergy, highBassEnergy/totalEnergy, lowMidEnergy/totalEnergy, midMidEnergy/totalEnergy, highMidEnergy/totalEnergy, lowHighEnergy/totalEnergy, midHighEnergy/totalEnergy, highHighEnergy/totalEnergy, airEnergy/totalEnergy });

	}
	
	//DO NOT USE DELETE USE FFTW_FREE TO FREE THE DATA
	fftw_destroy_plan(plan);
	fftw_free(in);
	fftw_free(out);

	tempsound->unlock(ptr1, ptr2, len1, len2); // Unlock the sound data after processing
	
	tempsound->release(); // Release the temporary sound object
	system->mixerResume(); // Resume the mixer after processing

	return bandHolder;
}

void FmodController::AnalyzeAudioOnline()
{
	bool playing = false;
	if (channel->isPlaying(&playing))
	{
		return;
	}
	float sampleRate = 48000.0f; // Default sample rate, can be updated based on the actual sound being played
	AudioBands bands;

	// Get the FFT data from the DSP
	system->update(); // Update the FMOD system to process audio and DSP
	FMOD_DSP_PARAMETER_FFT* fftData = nullptr;
	unsigned int length = 0; // will be filled with size of the fft data in bytes

	dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&fftData, &length,nullptr,0);//fills fftdata witht he data we want and lenght with the size


	if (fftData->length > 0)
	{
		int bins = fftData->length; // Number of frequency bins
		int windowSize = bins * 2; //creates the window size of 1024
		float peak = 0.0f; // the peak magnitude
		for (int i = 0; i < bins; i++)
		{
			float magnitude = fftData->spectrum[0][i];//fft mag for the bin we are in
			float freq = i * sampleRate / windowSize; //coverting the index toa f req
			
			//Assign magnitude to band
			if (freq >= 20.0f && freq <= 60.0f) {
				bands.sub = std::max(bands.sub, magnitude);
				peak = std::max(peak, bands.sub);
			}
			else if (freq >= 61.0f && freq <= 130.0f) {
				bands.bass = std::max(bands.bass, magnitude);
				peak = std::max(peak, bands.bass);
			}
			else if (freq >= 131.0f && freq <= 262.0f) {
				bands.highBass = std::max(bands.highBass, magnitude);
				peak = std::max(peak, bands.highBass);
			}
			else if (freq >= 263.0f && freq <= 523.0f) {
				bands.lowMid = std::max(bands.lowMid, magnitude);
				peak = std::max(peak, bands.lowMid);
			}
			else if (freq >= 524.0f && freq <= 1046.0f) {
				bands.midMid = std::max(bands.midMid, magnitude);
				peak = std::max(peak, bands.midMid);
			}
			else if (freq >= 1047.0f && freq <= 2093.0f) {
				bands.highMid = std::max(bands.highMid, magnitude);
				peak = std::max(peak, bands.highMid);
			}
			else if (freq >= 2094.0f && freq <= 4186.0f) {
				bands.lowHigh = std::max(bands.lowHigh, magnitude);
				peak = std::max(peak, bands.lowHigh);
			}
			else if (freq >= 4187.0f && freq <= 8000.0f) {
				bands.midHigh = std::max(bands.midHigh, magnitude);
				peak = std::max(peak, bands.midHigh);
			}
			else if (freq >= 8001.0f && freq <= 12000.0f) {
				bands.highHigh = std::max(bands.highHigh, magnitude);
				peak = std::max(peak, bands.highHigh);
			}
			else if (freq >= 12001.0f && freq <= 20000.0f) {
				bands.air = std::max(bands.air, magnitude);
				peak = std::max(peak, bands.air);
			}

		}
		//log f flattens the values before normalizing them in order to create better values tpo use for shader
		if (peak > 0.0f) // avoid division by zero
		{
			bands.sub = logf(bands.sub / peak + 1.0f);
			bands.bass = logf(bands.bass / peak + 1.0f);
			bands.highBass = logf(bands.highBass / peak + 1.0f);
			bands.lowMid = logf(bands.lowMid / peak + 1.0f);
			bands.midMid = logf(bands.midMid / peak + 1.0f);
			bands.highMid = logf(bands.highMid / peak + 1.0f);
			bands.lowHigh = logf(bands.lowHigh / peak + 1.0f);
			bands.midHigh = logf(bands.midHigh / peak + 1.0f);
			bands.highHigh = logf(bands.highHigh / peak + 1.0f);
			bands.air = logf(bands.air / peak + 1.0f);
		}
	}

	//// Print to console
	//std::cout
	//	<< "Sub: " << bands.sub << "  "
	//	<< "Bass: " << bands.bass << "  "
	//	<< "HighBass: " << bands.highBass << "  "
	//	<< "LowMid: " << bands.lowMid << "  "
	//	<< "MidMid: " << bands.midMid << "  "
	//	<< "HighMid: " << bands.highMid << "  "
	//	<< "LowHigh: " << bands.lowHigh << "  "
	//	<< "MidHigh: " << bands.midHigh << "  "
	//	<< "HighHigh: " << bands.highHigh << "  "
	//	<< "Air: " << bands.air << std::endl;


	PerframeAudioBand =  bands;

}

const AudioBands& FmodController::GetFrameAudioBand()
{
	return PerframeAudioBand;
}



FmodController::~FmodController()
{
	
	sounds->release();
	for (int i = 0; i < nameOfsounds.size(); i++)
	{
		nameOfsounds[i].clear();
	}
	dsp->release();
	system->close();
	system->release();
}
