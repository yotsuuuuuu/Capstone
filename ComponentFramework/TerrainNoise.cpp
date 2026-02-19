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
{    noiseGen.SetSeed(layerP.seed);
    noiseGen.SetNoiseType(ConvertNoiseType(layerP.type));

    noiseGen.SetFrequency(layerP.frequency);

    if (layerP.useFractal && layerP.fractal != FractalType::None) {
        noiseGen.SetFractalType(ConvertFractalType(layerP.fractal));
        noiseGen.SetFractalOctaves(layerP.octaves);
        noiseGen.SetFractalLacunarity(layerP.lacunarity);
        noiseGen.SetFractalGain(layerP.gain);
        noiseGen.SetFractalWeightedStrength(0.0f); // could change
        noiseGen.SetFractalPingPongStrength(layerP.bias);
    }
    else {
        noiseGen.SetFractalType(FastNoiseLite::FractalType_None);
    }

    if (layerP.useDomainWarp) {
        noiseGen.SetDomainWarpType(ConvertWarpType(layerP.domainWarp));
        noiseGen.SetDomainWarpAmp(layerP.warpAmp);
    }


    if (layerP.type == NoiseType::Cellular) {
        noiseGen.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
        noiseGen.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
        noiseGen.SetCellularJitter(1.0f);
    }
}

TerrainNoise::TerrainNoise(const TerrainPreset& preset)
	:	basePreset(preset.base), 
		mountainPreset(preset.mountains), 
		detailPreset(preset.detail),
		globalHeightScale(preset.globalHeightScale),
        concatenate(preset.concatenate),
        concatenateScale(preset.concatenateScale)
{
	InitializeNoiseLayer(basePreset, baseNoise);
	InitializeNoiseLayer(mountainPreset, mountainNoise);
	InitializeNoiseLayer(detailPreset, detailNoise);
}


float TerrainNoise::sample(float wX, float wZ) const
{
    float base = baseNoise.GetNoise(wX, wZ);
    float mountains = mountainNoise.GetNoise(wX, wZ);
    float detail = detailNoise.GetNoise(wX, wZ);

    // post-processing for terrain shaping
    float mask = std::clamp(base * 0.5f + 0.5f, 0.0f, 1.0f); // create a mask from base layer

    // combine layers with amplitude scaling
    float h = base *basePreset.amplitude;
    h += mountains * mask *mountainPreset.amplitude; // mountains only in base areas
    h += detail * detailPreset.amplitude * 0.25f; // subtle detail everywhere

    // apply additional shaping based on layer properties
    if (basePreset.exponent != 1.0f) {
        h = std::pow(std::max(0.0f, h), basePreset.exponent);
    }

    if (mountainPreset.ridge > 1.0f) {
        // additional ridge enhancement if needed
        h *= (1.0f + std::abs(mountains) * (mountainPreset.ridge - 1.0f));
    }

    if (concatenate) { return Concatenate(h); };

    return h * globalHeightScale;
}

int TerrainNoise::Concatenate(float h) const
{
    return (int)h * globalHeightScale;
}

