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
	//base.cellType = CellularType::Euclidian; // probably can avoid using cellular for base
	base.domainWarp = WarpType::None; // for the base layer it might be best to leave it off or if on keep low amp
	base.warpAmplitude = 0.2f; // keep low .2-.8

	// CONTINENTALNESS
	continentalness.type = NoiseType::Cellular; // cellular or value cubic seem to be good options
	continentalness.seed = 3847598; // idk hash the song
	continentalness.frequency = 0.005f; // for continentalness keep it low. 0.001-0.01
	continentalness.amplitude = 0.2f; // maybe even 0 since were really jsut using it to determine height ranges. maybe 0.2
	continentalness.fractal = FractalType::None; // anything but ridged
	continentalness.fractalOctaves = 1; // if using fractal, 3-5 octaves is good for continentalness (influenced by ??)
	continentalness.fractalWeightedStrength = 0.0f; // not needed for continentalness
	continentalness.gain = 0.005f; // keep very very very small 
	continentalness.cellType = CellularType::Euclidian; // try not to use hybrid or manhattan
	continentalness.returnType = ReturnType::Distance2Sub; // anything works here jsut keep in mind cell value will be sharp changes
	continentalness.cellularJitter = 1.0f; // keep around 1, 
	continentalness.domainWarp = WarpType::None; // not needed for continentalness
	

	// EROSION
	// maybe use this layer to add some erosion-like details to the terrain, such as small ridges or valleys.

	// PEAKS AND VALLEYS

}	
