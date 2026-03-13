#pragma once
#include <cstdint>
#include <vector>
#include "CoreStructs.h"
//#include "FmodController.h"

enum class NoiseType {
	Perlin = 0, // also creates smooth terrain, but can be a bit more "blobby" and less detailed than OpenSimplex2. good for rolling hills and gentle landscapes.
	OpenSimplex2, // it creates smooth, natural-looking terrain. more contrast between light and dark compared to perlin.
	Value, // square-ish, more blocky terrain. 
	Cubic, // similar to value but smoother transitions between values, more natural look while still maintaining some blockiness. 
	Cellular // globby , island-like terrain. dark massese with sharp white edges
};

enum class WarpType {
	None = 0, // no domain warping, just regular noise
	OpenSimplex2, // displaces the noise using another layer of OpenSimplex2 noise, can create complex and interesting terrain features. high amplitude creates distorded but still squareish patterns
	BasicGrid // displaces using a grid. high amplitude creates square patters
};

enum class FractalType {
    None = 0,
    FBm, // layers more noise on top of itself. each layer is more detail at smaller scale. high octaves can make it noisey
    PingPong, // inset mountains with dramatic peaks.
	Ridged // creates sharp bright peaks. good for mountains. high octaves can create more peaks but also more noise. 
};

enum class CellularType {
    None = 0,
    Euclidian, // generally gray network of white lines. some darker blobs
	EuclidianSq, // similar to euclidian but with sharper lines and more contrast between light and dark areas. good for more defined, blocky terrain features.
	Manhattan, // almost crystalline looking, with straight lines and sharp angles. 
    Hybrid // take manhattan and round some of the edges
};

enum class ReturnType {
    None = 0,
    Distance2, // similar to distance but less contrast, everything becomes lighter except with hybrid where its more contrast
    Distance2Add, // everything becomes lighter, good for more mountainous terrain
    Distance, //  default return 
    Distance2Div, // the brighest whites, sharpers lines
    Distance2Sub, // creates more contrast, more inset looking terrain
    Distance2Mul, //  like add but now everything is darker
    CellValue // blocky, no transition between values, blocky and sharp edges 
};



struct ProcessedAudio
{
    AudioBands avgBands;
    AudioBands maxBands;

	float averageLoudness = 0;
	float maxLoudness = 0;
	float tempo = 0;
	int songLength = 0; // based on number of windows
    uint32_t seed = 0;

    float bassAvgSum = 0.0f;
    float midAvgSum = 0.0f;
    float highAvgSum = 0.0f;
    
	float bassMaxSum = 0.0f;
	float midMaxSum = 0.0f;
	float highMaxSum = 0.0f;

};

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

	ProcessedAudio pAudio; // store processed audio data for use in noise generation

	void CreateFromAudio(std::vector<AudioBands> ab);
	ProcessedAudio GetLayerValuesFromAudio(std::vector<AudioBands> ab);
    NoiseType ChooseNoise(int layer, float value);
	FractalType ChooseFractal(int layer, float value);
    WarpType ChooseWarp(int layer, float value);
	CellularType ChooseCellular(int layer, float value);
	ReturnType ChooseReturn(int layer, float value);

    void CreateBase();
	void CreateContinentalness();
	void CreateErosion();
	void CreatePeaksValleys();

};
