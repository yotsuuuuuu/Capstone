#pragma once
#include "TerrainPreset.h"
#include "FastNoiseLite.h"

class TerrainNoise
{
public:
	TerrainNoise(const TerrainPreset& preset);
	float sample(float wX, float wZ) const;
	float advancedSample(float wX, float wZ) const;

	TerrainPreset terrainConfig;
	float EvaluateContinental(float c) const;
	int Concatenate(float h) const;
	int clamps(float h) const;
	float spike(float h) const;

private:

	NoiseLayerPreset basePreset, continentalPreset, erosionPreset, PVpreset;	// different noise layers
	FastNoiseLite baseNoise, continentalNoise, erosionNoise, PVnoise;		// unique noise generators for each layer

};

