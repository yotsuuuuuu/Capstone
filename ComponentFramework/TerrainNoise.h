#pragma once
#include "TerrainPreset.h"
#include "FastNoiseLite.h"

class TerrainNoise
{
public:
	TerrainNoise(const TerrainPreset& preset);
	float sample(float wX, float wZ) const;

	int Concatenate(float h) const;
	int clamps(float h) const;
	float spike(float h) const;

private:

	NoiseLayerPreset basePreset, mountainPreset, detailPreset;	// different noise layers
	FastNoiseLite baseNoise, mountainNoise, detailNoise;		// unique noise generators for each layer
	float globalHeightScale;
	bool concatenate = false;
	int  concatenateScale = 1;

};

