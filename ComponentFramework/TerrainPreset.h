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

enum class CellularType {
    None,
    Euclidian,
    EuclidianSq,
    Manhattan,
    Hybrid
};

enum class ReturnType {
    None,
    CellValue,
    Distance,
    Distance2,
	Distance2Add,
    Distance2Sub,
	Distance2Mul,
	Distance2Div
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


struct NoiseLayerPreset 
{
    NoiseType type = NoiseType::Perlin;
    uint32_t seed = 1337;
    float frequency = 0.01f;
    float amplitude = 1.0f;

    FractalType fractal = FractalType::None;
    int fractalOctaves = 1;
    float lacunarity = 0.1f;
    float gain = 0.1f;
    float fractalWeightedStrength = 0.0f;

    // cellular
    CellularType cellType = CellularType::None;
    ReturnType returnType = ReturnType::None;
    float cellularJitter = 1.0f;

    WarpType domainWarp = WarpType::None;
    float warpAmplitude = 0.2f;


};

struct TerrainPreset {
	// can be adjusted based on desired terrain features/ song style etc (+/- layers)

    int numLayers;
    std::vector<NoiseLayerPreset> layers; // probs not needed. will stick with 3 layers

    bool concatenate = false;

    float globalHeightScale = 5.0f;
    float exponent = 1.0f;
    float ridge = 1.0f;
    float bias = 1.0f;

    NoiseLayerPreset base;
    NoiseLayerPreset continentalness;
    NoiseLayerPreset erosion; // maybe ??
    NoiseLayerPreset peaksValleys; // maybe??

    void CreateFromAudio(AudioBands ab);
};
