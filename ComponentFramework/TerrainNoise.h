#pragma once
#include "TerrainPreset.h"
#include "FastNoiseLite.h"

class TerrainNoise
{
public:
	TerrainNoise(const TerrainPreset& preset);
	float sample(float wX, float wZ) const;

private:
	float evalLayer(const NoiseLayerPreset& layerP,
					const FastNoiseLite& noiseGen,
					float x, float z) const;

	NoiseLayerPreset basePreset, mountainPreset, detailPreset; // different noise layers
	FastNoiseLite baseNoise, mountainNoise, detailNoise;
	float globalHeightScale;
};

