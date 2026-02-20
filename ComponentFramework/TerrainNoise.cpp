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

static FastNoiseLite::CellularDistanceFunction ConvertCellularType(CellularType type) 
{
    switch (type) {
    case CellularType::Euclidian:   return FastNoiseLite::CellularDistanceFunction_Euclidean;
    case CellularType::EuclidianSq: return FastNoiseLite::CellularDistanceFunction_EuclideanSq;
    case CellularType::Manhattan:   return FastNoiseLite::CellularDistanceFunction_Manhattan;
    case CellularType::Hybrid:      return FastNoiseLite::CellularDistanceFunction_Hybrid;
    case CellularType::None:        return FastNoiseLite::CellularDistanceFunction_Euclidean;
    default:                        return FastNoiseLite::CellularDistanceFunction_Euclidean;
    }
}

static FastNoiseLite::CellularReturnType ConvertReturnType(ReturnType type)
{
    switch (type) {
    case ReturnType::CellValue:     return FastNoiseLite::CellularReturnType_CellValue;
    case ReturnType::Distance:      return FastNoiseLite::CellularReturnType_Distance;
    case ReturnType::Distance2:     return FastNoiseLite::CellularReturnType_Distance2;
    case ReturnType::None:          return FastNoiseLite::CellularReturnType_CellValue;
    default:                        return FastNoiseLite::CellularReturnType_CellValue;
    }
}

static void InitializeNoiseLayer(const NoiseLayerPreset& layerP, FastNoiseLite& noiseGen)
{    noiseGen.SetSeed(layerP.seed);
    noiseGen.SetNoiseType(ConvertNoiseType(layerP.type));

    noiseGen.SetFrequency(layerP.frequency);

    if (layerP.fractal != FractalType::None) {
        noiseGen.SetFractalType(ConvertFractalType(layerP.fractal));
        noiseGen.SetFractalOctaves(layerP.fractalOctaves);
        noiseGen.SetFractalLacunarity(layerP.lacunarity);
        noiseGen.SetFractalGain(layerP.gain);
        noiseGen.SetFractalWeightedStrength(layerP.fractalWeightedStrength); // could change
    }
    else {
        noiseGen.SetFractalType(FastNoiseLite::FractalType_None);
    }

    if (layerP.domainWarp != WarpType::None) {
        noiseGen.SetDomainWarpType(ConvertWarpType(layerP.domainWarp));
        noiseGen.SetDomainWarpAmp(layerP.warpAmplitude);
    }


    if (layerP.type == NoiseType::Cellular) {
        noiseGen.SetCellularDistanceFunction(ConvertCellularType(layerP.cellType));
        noiseGen.SetCellularReturnType(ConvertReturnType(layerP.returnType)); // this cann be changed for silly effects
        noiseGen.SetCellularJitter(layerP.cellularJitter);
    }
}

TerrainNoise::TerrainNoise(const TerrainPreset& preset)
	:	basePreset(preset.base)
{
	InitializeNoiseLayer(basePreset, baseNoise);
    terrainConfig = preset;
}


float TerrainNoise::sample(float wX, float wZ) const
{
    float base = baseNoise.GetNoise(wX, wZ);
    float continentalness = continentalNoise.GetNoise(wX, wZ);
    float detail = PVnoise.GetNoise(wX, wZ);

    // post-processing for terrain shaping
    //float mask = std::clamp(base * 0.5f + 0.5f, 0.0f, 1.0f); // create a mask from base layer

    // combine layers with amplitude scaling
    float h = base * basePreset.amplitude;

    //h += base * mask * basePreset.amplitude;
    //h += continentalness * continentalPreset.amplitude;

    // apply additional shaping based on layer properties
    if (terrainConfig.exponent != 1.0f) {
        h = std::pow(std::max(0.0f, h), terrainConfig.exponent);
    }

    // here is where i can check for modifiers
    if (terrainConfig.concatenate) { h = Concatenate(h); };

    h += (detail-0.5f * PVpreset.amplitude);
    return h * terrainConfig.globalHeightScale;
}


int TerrainNoise::Concatenate(float h) const
{
    int temp = (int)h;
    return temp;
}

