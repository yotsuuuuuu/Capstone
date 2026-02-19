#pragma once
#include <cstdint>
//#include <vector>
#include "FmodController.h"

enum class NoiseType {
    OpenSimplex2,
    Perlin,
    Cellular,
    Value,
    Cubic
};

enum class WarpType {
    None,
    OpenSimplex2,
	BasicGrid
};

enum class FractalType {
    None,
    FBm,
    Ridged,
    PingPong
};

// TODO: (andres) what does what
// length: num chunks
// frequency: how often the noise pattern repeats across the world (higher frequency = more variation in smaller areas)
// loudness (amplitude): how much the noise affects the height (higher amplitude = taller mountains/ deeper valleys)
// have different base presets (biomes) depending on what we read from the song
// 
// clamp the height can be one biome
// maybe have some layers that only affect the heightmap in certain height ranges (a layer that only adds detail to the mountains but doesn't affect the plains)
// highs can affect num of layers
//


//struct NoiseLayerPreset {
//    NoiseType type;
//    uint32_t seed;
//
//    float frequency; // the density of randomness in a given area (less -> more)
//    float amplitude; // the difference in height between top and bottm (less -> more)
//
//	bool useFractal;
//    FractalType fractal; // for fractal brownian motion (NEEDED FOR COMPLEX TERRAIN)
//    int octaves; // number of iteration / amount of times it compound on the base 
//    float lacunarity; //
//    float gain;
//
//	bool useDomainWarp;
//    WarpType domainWarp;
//    float warpAmp;
//
//    float exponent;
//    float ridge;
//    float bias;
//};

struct NoiseLayerPreset 
{
    NoiseType baseType = NoiseType::Perlin;
    uint32_t seed = 1337;
    float baseFrequency = 0.01f;
    float amplitude = 1.0f;

    FractalType fractal = FractalType::None;
    int fractalOctaves = 1.0f;
    float lacunarity = 0.1f;
    float gain = 0.1f;
    float fractalWeightedStrength = 0.0f;

    WarpType domainWarp = WarpType::None;
    float warpAmplitude = 0.2f;

public:
    void SetBaseType(NoiseType type_) { baseType = type_; }
    void SetSeed(uint32_t seed_) { seed = seed_; }

};

struct TerrainPreset {
	// can be adjusted based on desired terrain features/ song style etc (+/- layers)

    int numLayers;
    std::vector<NoiseLayerPreset> layers; // probs not needed. will stick with 3 layers

    bool concatenate = false;
    int  concatenateScale = 0;

    float exponent = 1.0f;
    float ridge = 1.0f;
    float bias = 1.0f;

    void CreateFromAudio(AudioBands ab);

    NoiseLayerPreset base;
    NoiseLayerPreset mountains;
    NoiseLayerPreset detail;
    NoiseLayerPreset continentalness;
    NoiseLayerPreset erosion; // maybe ??
    NoiseLayerPreset peaksValleys; // maybe??

    float globalHeightScale;
};
