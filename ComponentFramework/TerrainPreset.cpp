#include "TerrainPreset.h"

void TerrainPreset::CreateFromAudio(AudioBands ab)
{
	// need to check song length (length = WORLD_SIZE)
	// maybe intensity could dictate number of layers, but thats for later

	base = {
	NoiseType::OpenSimplex2,// noise type
	42,						// seed
	0.01f,					// frequency
	0.3f,					// amplitude
	false,					// fractal?
	FractalType::FBm,		// fractal type
	5,						// octaves
	2.0f,					// lacunarity
	0.5f,					// gain
	false,					// warp?
	WarpType::OpenSimplex2,	// warp type	
	15.0f,					// warp amplitude
	1.0f,					// exponent
	1.0f,					// ridge
	0.0f					// bias
	};

	mountains = {
	NoiseType::OpenSimplex2,// noise type
	42,						// seed
	0.01f,					// frequency
	0.3f,					// amplitude
	false,					// fractal?
	FractalType::FBm,		// fractal type
	5,						// octaves
	2.0f,					// lacunarity
	0.5f,					// gain
	false,					// warp?
	WarpType::OpenSimplex2,	// warp type	
	15.0f,					// warp amplitude
	1.0f,					// exponent
	1.0f,					// ridge
	0.0f					// bias
	};

	detail = {
	NoiseType::OpenSimplex2,// noise type
	42,						// seed
	0.01f,					// frequency
	0.3f,					// amplitude
	false,					// fractal?
	FractalType::FBm,		// fractal type
	5,						// octaves
	2.0f,					// lacunarity
	0.5f,					// gain
	false,					// warp?
	WarpType::OpenSimplex2,	// warp type	
	15.0f,					// warp amplitude
	1.0f,					// exponent
	1.0f,					// ridge
	0.0f					// bias
		};

}
