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
