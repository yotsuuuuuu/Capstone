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
    : basePreset(preset.base), continentalPreset(preset.continentalness), PVpreset(preset.peaksValleys)
{
    InitializeNoiseLayer(basePreset, baseNoise);
    InitializeNoiseLayer(continentalPreset, continentalNoise);
    InitializeNoiseLayer(PVpreset, PVnoise);
    terrainConfig = preset;
}

float TerrainNoise::sample(float wX, float wZ) const
{
    // get raw noise values
    float base = baseNoise.GetNoise(wX, wZ);
    float pv = PVnoise.GetNoise(wX, wZ);
    float continentalnessRaw = continentalNoise.GetNoise(wX, wZ);

    // map to 0-1 range
    float baseNorm = (base + 1.0f) * 0.5f;
    float pvNorm = (pv + 1.0f) * 0.5f;
    float continentalness = (continentalnessRaw + 1.0f) * 0.5f;

    // evaluate continental height
    float cv = EvaluateContinental(continentalness);

    // combine layers
    float h = cv * continentalPreset.amplitude;
    h += baseNorm * basePreset.amplitude;
    h += pvNorm * PVpreset.amplitude;

    // apply exponent (only to positive values)
    if (terrainConfig.exponent != 1.0f && h > VERY_SMALL) {
        float maxH = continentalPreset.amplitude + basePreset.amplitude + PVpreset.amplitude;
        float t = (h + 0.02f) / maxH;
        h = std::pow(t, terrainConfig.exponent) * maxH/1.3f; // can be changed
    }

   /* if (terrainConfig.exponent != 1.0f && h > VERY_SMALL) {
        h = std::pow(h, terrainConfig.exponent);
    }*/

    // apply global scale
    h *= terrainConfig.globalHeightScale;

    // truncate if needed
    if (terrainConfig.truncate) {
        return static_cast<float>(Truncate(h)); // round, really
    }

    return h;
}

float TerrainNoise::EvaluateContinental(float c) const
{
    // c is already clamped to 0-1
    c = std::clamp(c, 0.0f, 1.0f);

    // continental breakpoints (control points)
    struct ControlPoint {
        float continental;
        float height;
    };

    // define control points for smooth interpolation
    std::vector<ControlPoint> points = {
        {0.0f, 0.0f},  
        {0.2f, 0.0f}, 
        {0.3f, 1.0f},
        {0.5f, 2.0f},
        {0.8f, 5.0f},   
        {0.9f, 8.0f},   
        {1.0f, 11.0f}   
    };

    // find which segment c falls into
    for (size_t i = 0; i < points.size() - 1; i++) {
        if (c >= points[i].continental && c <= points[i + 1].continental) {
            float t = (c - points[i].continental) / (points[i + 1].continental - points[i].continental);
            // cubic interpolation for smoother transitions
            float t2 = t * t;
            float t3 = t2 * t;

            // hermite interpolation for smooth curves
            float height = points[i].height * (2.0f * t3 - 3.0f * t2 + 1.0f) +
                points[i + 1].height * (-2.0f * t3 + 3.0f * t2) +
                0.0f * (t3 - 2.0f * t2 + t) +  // tangent at start
                0.0f * (t3 - t2);               // tangent at end

            return height;
        }
    }

    return points.back().height;
}

int TerrainNoise::Truncate(float h) const
{
    int temp;// = (int)h;
    temp = static_cast<int>(std::round(h));
    return temp;
}
