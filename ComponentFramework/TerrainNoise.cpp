#pragma once
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
	case ReturnType::Distance2Add:  return FastNoiseLite::CellularReturnType_Distance2Add;
	case ReturnType::Distance2Sub:  return FastNoiseLite::CellularReturnType_Distance2Sub;
	case ReturnType::Distance2Mul:  return FastNoiseLite::CellularReturnType_Distance2Mul;
	case ReturnType::Distance2Div:  return FastNoiseLite::CellularReturnType_Distance2Div;
    case ReturnType::None:          return FastNoiseLite::CellularReturnType_CellValue;
    default:                        return FastNoiseLite::CellularReturnType_CellValue;
    }
}

static void InitializeNoiseLayer(const NoiseLayerPreset& layerP, FastNoiseLite& noiseGen)
{
    noiseGen.SetSeed(layerP.seed);
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
    : basePreset(preset.base), continentalPreset(preset.continentalness), erosionPreset(preset.erosion), PVpreset(preset.peaksValleys)
{
    InitializeNoiseLayer(basePreset, baseNoise);
    InitializeNoiseLayer(continentalPreset, continentalNoise);
    InitializeNoiseLayer(erosionPreset, erosionNoise);
    InitializeNoiseLayer(PVpreset, PVnoise);
    terrainConfig = preset;
}


float TerrainNoise::sample(float wX, float wZ) const
{
    float base = baseNoise.GetNoise(wX, wZ);
    float pv = PVnoise.GetNoise(wX, wZ);
    float continentalness = continentalNoise.GetNoise(wX, wZ);
    // post-processing for terrain shaping

    float h = base * basePreset.amplitude;


    //h += base * mask * basePreset.amplitude;
    //h += continentalness * continentalPreset.amplitude;

    // apply additional shaping based on layer properties
    if (terrainConfig.exponent != 1.0f) {
        if (h <= 0) { // h is negative just return 0
            //h = -std::pow(abs(h), terrainConfig.exponent);
            h = 0;
        }
        else {
            h = std::pow(abs(h), terrainConfig.exponent);
        }
    }

    //cv = continentalness;

    // here is where i can check for modifiers
    //h += cv * continentalPreset.amplitude;

    h += pv * PVpreset.amplitude;

    float cv = EvaluateContinental(continentalness);
	h += cv * continentalPreset.amplitude;

	h *= terrainConfig.globalHeightScale;


    if (terrainConfig.concatenate) { h = Concatenate(h); };

    if (h != h) {
        printf("null");
    }
    //h += (detail-0.5f * PVpreset.amplitude);
    //std::cout << h << std::endl;

    //h = base * basePreset.amplitude; // for just testing base

    return h;
}


float TerrainNoise::EvaluateContinental(float c) const
{
	c = std::clamp(c, 0.0f, 1.0f); // ensure c is between 0 and 1

    float percent;

    // continental value
    float cv1 = 0.2f;
	float cv2 = 0.5f;
	float cv3 = 0.8f;
	float cv4 = 1.0f;

	// height value (spline value)
	float sv1 = 2.0f;
	float sv2 = 3.0f;
	float sv3 = 8.0f;
	float sv4 = 11.0f;

    if (c <= 0.3f) {
        percent = c / 0.3f; // 0 to 1 as c goes from 0 to 0.3
        return percent * sv1; // scale to spline 1 to spline 2 
    }
    else if (c <= 0.5f) {
        percent = (c - 0.3f) / (0.5f - 0.3f); // 0 to 1 as c goes from 0.3 to 0.5
        return sv1 + percent * (sv2 - sv1); // scale to 2 to 5
    }
    else if (c <= 0.9f) {
        percent = (c - 0.5f) / (0.9f - 0.5f); // 0 to 1 as c goes from 0.5 to 0.9
        return sv2 + percent * (sv3 - sv2); // scale to 5 to 7
    }
    else {
        percent = (c - 0.9f) / (1.0f - 0.9f); // 0 to 1 as c goes from 0.9 to 1
        return sv3 + percent * (sv4 - sv3); // scale to 7 to 8
	}
}

int TerrainNoise::Concatenate(float h) const
{
    int temp = (int)h;
    return temp;
}
