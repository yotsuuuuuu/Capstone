#pragma once
#include "TerrainPreset.h"
#include "FastNoiseLite.h"

class TerrainNoise
{
public:
	TerrainNoise(const TerrainPreset& preset);
	float sample(float wX, float wZ) const;

	TerrainPreset terrainConfig;
	float EvaluateContinental(float c) const;
	int Truncate(float h) const;

private:

	NoiseLayerPreset basePreset, continentalPreset, PVpreset;	// different noise layers
	FastNoiseLite baseNoise, continentalNoise, PVnoise;		// unique noise generators for each layer

};
