#include "TerrainNoise.h"

static FastNoiseLite::NoiseType ConvertNoiseType(NoiseType type)
{
	switch (type) {
	case NoiseType::OpenSimplex2:	return FastNoiseLite::NoiseType_OpenSimplex2;
	case NoiseType::Perlin:			return FastNoiseLite::NoiseType_Perlin;
	case NoiseType::Cellular:		return FastNoiseLite::NoiseType_Cellular;
	case NoiseType::Value:			return FastNoiseLite::NoiseType_Value;
	case NoiseType::Cubic:			return FastNoiseLite::NoiseType_ValueCubic;
	default:						return FastNoiseLite::NoiseType_OpenSimplex2;
	}
}

static FastNoiseLite::DomainWarpType ConvertWarpType(WarpType type)
{
	switch (type) {
	case WarpType::OpenSimplex2:		return FastNoiseLite::DomainWarpType_OpenSimplex2;
	case WarpType::BasicGrid:			return FastNoiseLite::DomainWarpType_BasicGrid;
	case WarpType::None:				return FastNoiseLite::DomainWarpType_OpenSimplex2; // no warp default to something || this shouldnt ever be used
	default:							return FastNoiseLite::DomainWarpType_OpenSimplex2;
	}
}

static FastNoiseLite::FractalType ConvertFractalType(FractalType type)
{
	switch (type) {
	case FractalType::FBm:			return FastNoiseLite::FractalType_FBm;
	case FractalType::Ridged:		return FastNoiseLite::FractalType_Ridged;
	case FractalType::PingPong:		return FastNoiseLite::FractalType_PingPong;
	case FractalType::None:			return FastNoiseLite::FractalType_None;
	default:						return FastNoiseLite::FractalType_None;
	}
}

static void InitializeNoiseLayer(const NoiseLayerPreset& layerP, FastNoiseLite& noiseGen)
{
	noiseGen.SetSeed(layerP.seed);
	noiseGen.SetFrequency(layerP.frequency);
	noiseGen.SetNoiseType(ConvertNoiseType(layerP.type));

	noiseGen.SetFractalType(ConvertFractalType(layerP.fractal));
	noiseGen.SetFractalOctaves(layerP.octaves);
	noiseGen.SetFractalLacunarity(layerP.lacunarity);
	noiseGen.SetFractalGain(layerP.gain);

	if (layerP.useDomainWarp) {
		noiseGen.SetDomainWarpType(ConvertWarpType(layerP.domainWarp));
		noiseGen.SetDomainWarpAmp(layerP.warpAmplitude);
	}
}

TerrainNoise::TerrainNoise(const TerrainPreset& preset)
	:	basePreset(preset.base), 
		mountainPreset(preset.mountains), 
		detailPreset(preset.detail),
		globalHeightScale(preset.globalHeightScale)
{
	InitializeNoiseLayer(basePreset, baseNoise);
	InitializeNoiseLayer(mountainPreset, mountainNoise);
	InitializeNoiseLayer(detailPreset, detailNoise);
}
{
}

float TerrainNoise::sample(float wX, float wZ) const
{
	return 0.0f;
}

float TerrainNoise::evalLayer(const NoiseLayerPreset& layerP, FastNoiseLite& noiseGen, float x, float z) const
{
	return 0.0f;
}
