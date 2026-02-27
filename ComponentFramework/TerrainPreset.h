#pragma once
#include <cstdint>
//#include <vector>
#include "FmodController.h"

enum class NoiseType {
	OpenSimplex2, // it creates smooth, natural-looking terrain. more contrast between light and dark compared to perlin.
	Perlin, // also creates smooth terrain, but can be a bit more "blobby" and less detailed than OpenSimplex2. good for rolling hills and gentle landscapes.
	Cellular, // globby , island-like terrain. dark massese with sharp white edges
	Value, // square-ish, more blocky terrain. 
	Cubic // similar to value but smoother transitions between values, more natural look while still maintaining some blockiness. 
};

enum class WarpType {
	None, // no domain warping, just regular noise
	OpenSimplex2, // displaces the noise using another layer of OpenSimplex2 noise, can create complex and interesting terrain features. high amplitude creates distorded but still squareish patterns
	BasicGrid // displaces using a grid. high amplitude creates square patters
};

enum class FractalType {
    None,
    FBm, // layers more noise on top of itself. each layer is more detail at smaller scale. high octaves can make it noisey
	Ridged, // creates sharp bright peaks. good for mountains. high octaves can create more peaks but also more noise. 
    PingPong // inset mountains with dramatic peaks.
};

enum class CellularType {
    None,
    Euclidian, // generally gray network of white lines. some darker blobs
	EuclidianSq, // similar to euclidian but with sharper lines and more contrast between light and dark areas. good for more defined, blocky terrain features.
	Manhattan, // almost crystalline looking, with straight lines and sharp angles. 
    Hybrid // take manhattan and round some of the edges
};

enum class ReturnType {
    None,
    CellValue, // blocky, no transition between values, blocky and sharp edges 
    Distance, //  default return 
    Distance2, // similar to distance but less contrast, everything becomes lighter except with hybrid where its more contrast
    Distance2Add, // everything becomes lighter, good for more mountainous terrain
    Distance2Sub, // creates more contrast, more inset looking terrain
    Distance2Mul, //  like add but now everything is darker
    Distance2Div // the brighest whites, sharpers lines
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
	NoiseType type = NoiseType::Perlin; // the type of noise to generate for this layer. 
	uint32_t seed = 1337; // the seed for the noise generation, can be random or based on the song
    float frequency = 0.01f; // how often the noise pattern repeats across the world (higher frequency = more variation in smaller areas)
    float amplitude = 1.0f; // how much the noise affects the height (higher amplitude = taller mountains/ deeper valleys)

    FractalType fractal = FractalType::None;
    int fractalOctaves = 3; // number of layers of noise to combine, higher values create more detailed terrain
    float lacunarity = 2.0f; // controls the frequency of each octave, higher values create more variation
    float gain = 0.5f; // gain controls how much each octave contributes to the final noise, higher gain creates sharper peaks but can also make it noisier.
    float fractalWeightedStrength = 0.0f; // fractal weighted strength can be used to create more dramatic peaks by increasing the influence of higher octaves.

    // cellular
    CellularType cellType = CellularType::Euclidian; // the type of cellular noise to generate
    ReturnType returnType = ReturnType::Distance; // the type of value to return from the cellular noise
    float cellularJitter = 1.0f; // controls the randomness of the cell positions (whites out at 8/-8, past 1.5 you start seeing jarring squares, 0 is a perfectly aligned grid)

	WarpType domainWarp = WarpType::None; // the type of domain warping to apply to the noise, which can create more complex and interesting terrain
    float warpAmplitude = 1.0f; // controls the intensity of the domain warping


};

struct TerrainPreset {
	// can be adjusted based on desired terrain features/ song style etc (+/- layers)

    int numLayers;
    std::vector<NoiseLayerPreset> layers; // probs not needed. will stick with 3 layers

    bool concatenate = false;

    float globalHeightScale = 5.0f;
	float exponent = 1.0f; // creates more dramatic terrain by exaggerating height differences. >1 will make mountains taller and valleys deeper, 0-1 will create a more flattened terrain.

    NoiseLayerPreset base;
    NoiseLayerPreset continentalness;
    NoiseLayerPreset erosion; // maybe ??
    NoiseLayerPreset peaksValleys; // maybe??

    void CreateFromAudio(AudioBands ab);
};
