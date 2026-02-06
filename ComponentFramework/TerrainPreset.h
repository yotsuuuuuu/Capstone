#pragma once
#include <cstdint>

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

// what does what
// length: num chunks
// frequency: how often the noise pattern repeats across the world (higher frequency = more variation in smaller areas)
// loudness (amplitude): how much the noise affects the height (higher amplitude = taller mountains/ deeper valleys)
// have different base presets (biomes) depending on what we read from the song
// 
// clamp the height can be one biome
// maybe have some layers that only affect the heightmap in certain height ranges (a layer that only adds detail to the mountains but doesn't affect the plains)
// highs can affect num of layers
//


struct NoiseLayerPreset {
    NoiseType type;
    uint32_t seed;

    float frequency;
    float amplitude;

	bool useFractal;
    FractalType fractal;
    int octaves;
    float lacunarity;
    float gain;

	bool useDomainWarp;
    WarpType domainWarp;
    float warpFrequency;
    float warpAmplitude;

    float exponent;
    float ridge;
    float bias;
};

struct TerrainPreset {
	// can be adjusted based on desired terrain features/ song style etc (+/- layers)
    NoiseLayerPreset base;
    NoiseLayerPreset mountains;
    NoiseLayerPreset detail;

    float globalHeightScale;
};
