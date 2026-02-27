#include "TerrainPreset.h"

void TerrainPreset::CreateFromAudio(AudioBands ab)
{
	// need to check song length (length = WORLD_SIZE)

	// BASE 
	// this is the main shape of the terrain, so maybe it should be more influenced by the song
	base.type = NoiseType::OpenSimplex2; // any works
	base.seed = 3847598; // idk hash the song
	base.frequency = 0.009f; // for base keep it very very low. 0.001-0.01
	base.amplitude = 5.0f; // 5 is a good starting point. nothing more than 10
	base.fractal = FractalType::None; // for base DO NOT USE PINGPONG OR RIDGED. its too crazy.
	base.fractalOctaves = 1; // if using fractal, 3-5 octaves is good for base (influenced by ??)
	base.fractalWeightedStrength = 0.0f; // not needed for base
	base.gain = 0.5f; // if using fractal, 0.3-0.5 is good for base
	base.domainWarp = WarpType::None; // for the base layer it might be best to leave it off or if on keep low amp
	base.warpAmplitude = 0.2f; // keep low .2-.8


}	
