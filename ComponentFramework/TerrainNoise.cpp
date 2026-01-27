#include "TerrainNoise.h"
#include <algorithm>

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
	case WarpType::None:				return FastNoiseLite::DomainWarpType_OpenSimplex2; // no warp default to something. this should never be used
	default:							return FastNoiseLite::DomainWarpType_OpenSimplex2; // same with this
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


float TerrainNoise::sample(float wX, float wZ) const
{

	float base = evalLayer(basePreset, baseNoise, wX, wZ);
	float mountains = evalLayer(mountainPreset, mountainNoise, wX, wZ);
	float detail = evalLayer(detailPreset, detailNoise, wX, wZ);

	float mask = std::clamp(base * 0.5f + 0.5f, 0.0f, 1.0f); // create a mask from base layer
	
	float h = base;
	h += mountains * mask; // apply mountains modulated by base mask
	h += detail * 0.25f; // add some detail

	return h *= globalHeightScale;

}

float TerrainNoise::evalLayer(const NoiseLayerPreset& layerP, const FastNoiseLite& noiseGen, float x, float z) const
{

	// all of this is temporary filler, can be adjusted later for what style() we are looking for. handles different behaviours and landscapes by modifying the sampled noise value
	float height = 0.0f;
	float amplitude = layerP.amplitude;
	float frequency = layerP.frequency;

	for (int i = 0; i < layerP.octaves; i++) { // do this for each octave (compounds the noise each time)
		float nx = x * frequency;
		float nz = z * frequency;

		if (layerP.useDomainWarp) {
			float warp = noiseGen.GetNoise(nx * layerP.warpFrequency, nz * layerP.warpFrequency);
			nx += warp * layerP.warpAmplitude;
			nz += warp * layerP.warpAmplitude;
		}

		float noiseValue = noiseGen.GetNoise(nx, nz); // sample the noise
		if (layerP.fractal == FractalType::Ridged) {
			noiseValue = 1.0f - fabs(noiseValue); // make ridges
			noiseValue *= noiseValue; // square to sharpen
			noiseValue *= layerP.ridge; // apply ridge factor
		}
		else if (layerP.fractal == FractalType::PingPong) {
			noiseValue = noiseValue * 2.0f; // 
			if (noiseValue < 0.0f) noiseValue = -noiseValue;
			if (noiseValue > 1.0f) noiseValue = 2.0f - noiseValue;
			noiseValue *= layerP.bias;
		}// if fractal is fBm do nothing special

		height += noiseValue * amplitude;
		amplitude *= layerP.gain;
		frequency *= layerP.lacunarity;
	}

	return height;
}
