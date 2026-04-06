#include "TerrainPreset.h"

void TerrainPreset::CreateFromAudio(std::vector<AudioBands> ab)
{
	pAudio = GetLayerValuesFromAudio(ab);

	// scale high frequencies to a more usable range (since they tend to be very low) and clamp to 1.0f
	//float highScaled = std::min(pAudio.highAvgSum * 100.0f, 1.0f); 
	float highScaled = std::min(pAudio.highAvgSum * 10.0f, 1.0f); 
    //float highScaled = pAudio.highAvgSum * 100.0f;
    float midScaled = std::min(pAudio.midAvgSum * 2.0f, 1.0f);
	float bassScaled = pAudio.bassAvgSum; // bass is usually already in a good range so no need to scale much, maybe just a little

	CreateBase();
	CreatePeaksValleys();
	//CreateErosion();
	CreateContinentalness();
	
	exponent =  1 + (highScaled*2);
	std::cout <<"Terrain preset - exponent: " << exponent << std::endl;

	float midDominance = pAudio.midAvgSum * 1.2f; 
	float bassInfluence = pAudio.bassAvgSum / 1.4f; // if mids are significantly higher than bass, consider them dominant
	bool midsDominate = midDominance > bassInfluence;
	bool highsDominate = highScaled > 0.5f;

	// if mids are dominant, truncate layers for more complex terrain. 
	//concatenate = (pAudio.midAvgSum*1.3 > pAudio.bassAvgSum/1.6f) && (pAudio.midAvgSum > pAudio.highAvgSum); 
	truncate = midsDominate || (pAudio.midAvgSum > 0.3f && highScaled > 0.4f);

	globalHeightScale = 3.0f + (pAudio.bassAvgSum * 10.0f); // bass can influence overall height since its usually the highest and can create more dramatic terrain with higher values

    std::cout <<"Terrain Preset - global height scale: " << globalHeightScale << std::endl;

    WORLD_SIZE = std::clamp(20 + (pAudio.songLength / 200), 25, 32);

    int maxActors = (WORLD_SIZE * WORLD_SIZE) * 2.5; // 2x world size amount of actors (2 per chunk roughly)
    //int totalActors = 0;
    actorAmount.lights = std::clamp(40 + static_cast<int>(pAudio.maxBands.lowMid), 40, maxActors / 2); // keep between 40-half max
    actorAmount.tree1 = std::clamp(static_cast<int>(1 + (pAudio.bassAvgSum * (maxActors / 6))), 1, maxActors / 6);
    actorAmount.tree2 = std::clamp(static_cast<int>(1 + (pAudio.bassAvgSum + pAudio.highAvgSum * 2)) * (maxActors / 6), 1, maxActors / 6);
    actorAmount.rock1 = std::clamp(static_cast<int>(1 + (pAudio.midAvgSum * (maxActors / 4))), 1, maxActors / 4);
    actorAmount.rock2 = std::clamp(static_cast<int>(1 + (pAudio.midAvgSum + pAudio.highAvgSum * 2) * (maxActors / 6)), 1, maxActors / 6);


    int totalActors = actorAmount.lights + actorAmount.rock1 + actorAmount.rock2 + actorAmount.tree1 + actorAmount.tree2;

    if (totalActors > maxActors) { // if too many take away from lights
        int temp = totalActors - maxActors;
        actorAmount.lights -= temp;
    }
    else if(totalActors < maxActors){ // if too little add to lights
        int temp = maxActors - totalActors;
        actorAmount.lights += temp;
    }
    totalActors = actorAmount.lights + actorAmount.rock1 + actorAmount.rock2 + actorAmount.tree1 + actorAmount.tree2;
    actorAmount.totalActors = totalActors;

    magicNumber = (pAudio.seed/2) + (pAudio.highMaxSum * 100);
	// TODO: (andres) concatenation (need more modifiers ex. concat can be to certain fractions instead of whole)
	// TODO: (andres) continentallness heights
	// TODO: (andres) randomize actor placement (lights, etc) ( x % worldsize, then x % chunksize)


	// EROSION
	// maybe use this layer to add some erosion-like details to the terrain, such as small ridges or valleys.

	// PEAKS AND VALLEYS
	//peaksValleys.type = NoiseType::Cubic; // any works
}

ProcessedAudio TerrainPreset::GetLayerValuesFromAudio(std::vector<AudioBands> ab)
{

	ProcessedAudio pa;

	for (auto& band : ab) {
		pa.avgBands.sub += band.sub;
		pa.avgBands.bass += band.bass;
		pa.avgBands.highBass += band.highBass;
		pa.avgBands.lowMid += band.lowMid;
		pa.avgBands.midMid += band.midMid;
		pa.avgBands.highMid += band.highMid;
		pa.avgBands.lowHigh += band.lowHigh;
		pa.avgBands.midHigh += band.midHigh;
		pa.avgBands.highHigh += band.highHigh;
		pa.avgBands.air += band.air;
		pa.averageLoudness += band.sub + band.bass + band.highBass + band.lowMid + band.midMid + band.highMid + band.lowHigh + band.midHigh + band.highHigh +band.air;

	}

	// need to actually hash the song name or something to get a unique seed for each song
	//pa.seed = std::hash<std::string>{}("song name or something"); // placeholder need a way to get the song name/path
	pa.songLength = static_cast<int>(ab.size());

	// copy before averaging out
	pa.maxBands = pa.avgBands; 
	pa.maxLoudness = pa.averageLoudness; // this just becomes the song length, cause duh

	// of note: bass tends to be the highest followed by mid and then high in dead last

	pa.avgBands.sub /= float(pa.songLength);
	pa.avgBands.bass /= float(pa.songLength);
	pa.avgBands.highBass /= float(pa.songLength);
	pa.avgBands.lowMid /= float(pa.songLength);
	pa.avgBands.midMid /= float(pa.songLength);
	pa.avgBands.highMid /= float(pa.songLength);
	pa.avgBands.lowHigh /= float(pa.songLength);
	pa.avgBands.midHigh /= float(pa.songLength);
	pa.avgBands.highHigh /= float(pa.songLength); // this one is always SUPER low (e-05 etc) so maybe when handling it *100 at LEAST
	pa.avgBands.air /= float(pa.songLength);
	pa.averageLoudness /= float(pa.songLength); // this just becomes 1 ;-;


	pa.bassAvgSum = pa.avgBands.bass + pa.avgBands.sub + pa.avgBands.highBass; 
	pa.midAvgSum = pa.avgBands.lowMid + pa.avgBands.midMid + pa.avgBands.highMid;
	pa.highAvgSum = pa.avgBands.lowHigh + pa.avgBands.midHigh + pa.avgBands.highHigh;

	// dunno why but they exist and they might be useful for some modifiers or something so might as well calculate them
	pa.bassMaxSum = pa.maxBands.bass + pa.maxBands.sub + pa.maxBands.highBass;
	pa.midMaxSum = pa.maxBands.lowMid + pa.maxBands.midMid + pa.maxBands.highMid;
	pa.highMaxSum = pa.maxBands.lowHigh + pa.maxBands.midHigh + pa.maxBands.highHigh;

	pa.seed = pa.songLength * 1.5f * ((pa.highMaxSum * pa.bassMaxSum * pa.midMaxSum) * (pa.avgBands.air * 10 + 1)); // SEED

	return pa;
}

NoiseType TerrainPreset::ChooseNoise(int layer, float value)
{
    // Define available noise types per layer
    std::vector<NoiseType> availableNoise;

    switch (layer) {
    case 0: // base layer - biggest impact on overall terrain shape
        availableNoise = {
            NoiseType::Perlin,      // classic smooth hills
            NoiseType::OpenSimplex2 // more natural shapes
        };
        break;

    case 1: // peaks and valleys - need dramatic shapes
    case 2: // erosion - avoid cellular for natural erosion patterns
        availableNoise = {
            NoiseType::Perlin,       // rolling hills
            NoiseType::OpenSimplex2, // natural peaks
            NoiseType::Value,        // blocky peaks
            NoiseType::Cubic         // smooth valleys
            // cellular excluded - creates unnatural erosion
        };
        break;

    case 3: // continentalness - broad landmass shapes
        availableNoise = {
            NoiseType::Value,        // blocky continents
            NoiseType::Cubic,        // smooth continents
            NoiseType::Cellular      // island clusters
        };
        break;

    default:
        availableNoise = {
            NoiseType::Perlin,
            NoiseType::OpenSimplex2,
            NoiseType::Cellular,
            NoiseType::Value,
            NoiseType::Cubic
        };
        break;
    }

    // clamp value to 0-0.999 to avoid out-of-bounds
    value = std::clamp(value, 0.0f, 0.999f);

    // map value to index in available range
    int index = static_cast<int>(value * static_cast<float>(availableNoise.size()));

    return availableNoise[index];
}

FractalType TerrainPreset::ChooseFractal(int layer, float value)
{
    // define available fractal types per layer
    std::vector<FractalType> availableFractals;

    switch (layer) {
    case 0: // base layer - avoid Ridged (too sharp for continents)
        availableFractals = {
            FractalType::None,    // smooth, no fractal
            FractalType::FBm,     // standard fractal (good for hills)
            FractalType::PingPong // creates interesting plateaus
            // [x] [x] [x] [ ]    // 3 fractals available (No Ridged)
        };
        break;

    case 1: // peaks and valleys - dramatic terrain needs Ridged
        availableFractals = {
            FractalType::None,    // some areas can be smooth
            FractalType::FBm,     // rolling hills
            //FractalType::Ridged,  // sharp mountain peaks
            FractalType::PingPong // chaotic, rocky terrain
            // [x] [x] [x] [x]    // all 4 fractals available
        };
        break;

    case 2: // erosion - need organic patterns
        availableFractals = {
            FractalType::FBm,     // gentle erosion
            FractalType::Ridged,  // sharp erosion gullies
            FractalType::PingPong // complex erosion patterns
        };
        break;

    case 3: // continentalness - broad shapes
        availableFractals = {
            FractalType::None,    // smooth continents
            FractalType::FBm      // slightly varied continents
        };
        break;

    default:
        availableFractals = {
            FractalType::None,
            FractalType::FBm,
            FractalType::Ridged,
            FractalType::PingPong
        };
        break;
    }

    // clamp value to 0-0.999 to avoid out-of-bounds
    value = std::clamp(value, 0.0f, 0.999f);

    // map value to index in available range
    int index = static_cast<int>(value * static_cast<float>(availableFractals.size()));

    return availableFractals[index];
}

WarpType TerrainPreset::ChooseWarp(int layer, float value)
{
    // define available warp types per layer
    std::vector<WarpType> availableWarps;

    switch (layer) {
    case 0: // base layer - can use any warp
        availableWarps = {
            WarpType::None,          // no warping
            WarpType::OpenSimplex2,  // organic warping
            WarpType::BasicGrid      // grid-based warping
        };
        break;

    case 1: // peaks and valleys - avoid BasicGrid (looks unnatural)
        availableWarps = {
            WarpType::None,          // keep shape clean
            WarpType::OpenSimplex2   // organic warping only
            // [x] [x] [ ]           // 2 warp types available
        };
        break;

    case 2: // erosion - needs organic warping
    case 3: // continentalness - needs subtle warping
        availableWarps = {
            WarpType::OpenSimplex2,  // organic warping
            WarpType::BasicGrid      // grid warping for canyons
            // [ ] [x] [x]           // last 2 warp types available
        };
        break;

    default:
        availableWarps = {
            WarpType::None,
            WarpType::OpenSimplex2,
            WarpType::BasicGrid
        };
        break;
    }

    // clamp value to 0-0.999 to avoid out-of-bounds
    value = std::clamp(value, 0.0f, 0.999f);

    // map value to index in available range
    int index = static_cast<int>(value * static_cast<float>(availableWarps.size()));

    return availableWarps[index];
}

CellularType TerrainPreset::ChooseCellular(int layer, float value)
{
    // define available cellular types per layer
    std::vector<CellularType> availableCellular;

    switch (layer) {
    case 0: // base layer - not typically used
    case 1: // peaks and valleys - avoid Hybrid (too chaotic)
        availableCellular = {
            CellularType::Euclidian,   // smooth cell boundaries
            CellularType::EuclidianSq, // slightly sharper
            CellularType::Manhattan    // grid-like cells
        };
        break;

    case 2: // erosion - can use more variety
    case 3: // continentalness - needs cell clustering
        availableCellular = {
            CellularType::Euclidian,   // natural clusters
            CellularType::EuclidianSq, // defined boundaries
            CellularType::Manhattan,   // city-block clusters
            CellularType::Hybrid       // complex shapes
        };
        break;

    default:
        availableCellular = {
            CellularType::Euclidian,
            CellularType::EuclidianSq,
            CellularType::Manhattan,
            CellularType::Hybrid
        };
        break;
    }

    // clamp value to 0-0.999 to avoid out-of-bounds
    value = std::clamp(value, 0.0f, 0.999f);

    // map value to index in available range
    int index = static_cast<int>(value * static_cast<float>(availableCellular.size()));

    return availableCellular[index];
}

ReturnType TerrainPreset::ChooseReturn(int layer, float value)
{
    // define available return types per layer
    std::vector<ReturnType> availableReturns;

    switch (layer) {
    case 0: // base layer - not typically used
        value = 0; // not needed
        availableReturns = {
            ReturnType::Distance,      // basic distance field
            ReturnType::Distance2,     // squared distance
            ReturnType::CellValue      // cell indices
        };
        break;

    case 1: // peaks and valleys - need distance-based returns
        availableReturns = {
            ReturnType::Distance2,       // sharp boundaries
            ReturnType::Distance2Add,    // distance + cell value
            ReturnType::Distance2Sub     // distance - cell value
        };
        break;

    case 2: // erosion - can use more variety
    case 3: // continentalness - needs cell value for continents
        availableReturns = {
            ReturnType::CellValue,       // clear continent boundaries
            ReturnType::Distance,        // smooth transitions
            ReturnType::Distance2,       // sharper edges
            ReturnType::Distance2Add     // combined effects
        };
        break;

    default:
        availableReturns = {
            ReturnType::CellValue,
            ReturnType::Distance,
            ReturnType::Distance2,
            ReturnType::Distance2Add
        };
        break;
    }

    // clamp value to 0-0.999 to avoid out-of-bounds
    value = std::clamp(value, 0.0f, 0.999f);

    // map value to index in available range
    int index = static_cast<int>(value * static_cast<float>(availableReturns.size()));

    return availableReturns[index];
}



// maybe change to no arguments and make it one big call
// value can be used to further modify things
void TerrainPreset::CreateBase()
{
	// int for layer is 0 = base

	AudioBands avgBands = pAudio.avgBands;

	// base could use some sprinkling from highs for fractals or something
	base.type = ChooseNoise(0, avgBands.bass); // for base decide it with bass
	base.seed = pAudio.seed * (pAudio.bassAvgSum + pAudio.highAvgSum + 1); // vary seed slightly for each layer
	base.frequency = 0.001f + (avgBands.sub * 0.03f); // for base keep it very very low. 0.001-0.01
	base.amplitude = 2.0f + (avgBands.highBass * 3.0f); // 5 is a good starting point. nothing more than 10
	base.fractal = ChooseFractal(0, pAudio.bassAvgSum * 2); // for base DO NOT USE PINGPONG OR RIDGED. its too crazy.

	if (base.fractal != FractalType::None) {
		base.fractalOctaves = 1 + std::round((pAudio.highAvgSum) * 20); // influenced by highs (super low so *20)
		base.gain = 0.1f + ((pAudio.bassAvgSum) * 0.2f); // if using fractal, 0.3-0.5 is good for base
		base.lacunarity = 2.0f + (pAudio.bassAvgSum * 0.5f);
		//base.fractalWeightedStrength = (pAudio.highAvgSum) * 2.0f; // highs since they are so low
	}

	// no need for cell

	base.domainWarp = ChooseWarp(0, pAudio.bassAvgSum); // highs because why not ??
	if (base.domainWarp != WarpType::None) {
		base.warpAmplitude = 0.2f + (avgBands.highBass * 0.6f); // keep low .2-.8
	}
}

void TerrainPreset::CreatePeaksValleys()
{
	AudioBands avgBands = pAudio.avgBands;
	peaksValleys.type = ChooseNoise(1, pAudio.highAvgSum); // avg sum cause alone they tiny 
	peaksValleys.seed = pAudio.seed * (pAudio.midAvgSum + pAudio.highAvgSum + 1); // vary seed slightly for each layer
	peaksValleys.frequency = 0.001f + (pAudio.highAvgSum * 0.2f); // for peaks and valleys we want a bit more frequency than base to add some variation, but not too much that it becomes noisy
	peaksValleys.amplitude = 0.3f + (pAudio.midAvgSum * 0.5f); // keep it lower than base since its just adding details on top
	peaksValleys.fractal = ChooseFractal(1, pAudio.highAvgSum); // for peaks and valleys, pingpong and ridged can create some nice dramatic peaks
	
	if (peaksValleys.fractal != FractalType::None) {
		peaksValleys.fractalOctaves = 1 + std::round((pAudio.midAvgSum) * 3); // influenced by mids since they are in the middle
		peaksValleys.gain = 0.3f + ((pAudio.midAvgSum) * 0.4f); // if using fractal, 0.3-0.7 is good for peaks and valleys
		peaksValleys.lacunarity = 2.0f + (pAudio.midAvgSum * 0.5f);
		peaksValleys.fractalWeightedStrength = (pAudio.highAvgSum) * 1.5f; 
	}

	peaksValleys.domainWarp = ChooseWarp(1, pAudio.midAvgSum + pAudio.highMaxSum); 
	if (peaksValleys.domainWarp != WarpType::None) {
		peaksValleys.warpAmplitude = 0.2f + (pAudio.midAvgSum * 0.6f); // keep low .2-.8
	}
}

void TerrainPreset::CreateErosion()
{

}

void TerrainPreset::CreateContinentalness()
{
	//AudioBands avgBands = pAudio.avgBands;
	continentalness.type = ChooseNoise(3, (pAudio.midAvgSum + pAudio.highAvgSum)); // mids and highs since they can add some variation without completely changing the overall shape of the terrain
	continentalness.seed = pAudio.seed * (pAudio.midAvgSum + pAudio.highAvgSum + 1); // vary seed slightly for each layer
	//continentalness.frequency = 0.002f * (avgBands.sub * 0.008f); // almost lower than base
	//continentalness.amplitude = 0.1f + (pAudio.bassAvgSum * 0.33f); 
	//
	////continentalness.fractal = ChooseFractal(3,pAudio.bassMaxSum * avgBands.midHigh * 0.01f);
	//if (continentalness.fractal != FractalType::None) {
	//	continentalness.fractalOctaves = 1 + std::round((pAudio.midAvgSum) * 1.2f); // barely any octaves 
	//	continentalness.gain = 0.005f + (pAudio.midAvgSum * 0.05f); // keep very very very small 
	//	continentalness.lacunarity = 1.7f + (pAudio.midAvgSum * 0.6f);
	//	continentalness.fractalWeightedStrength = (pAudio.midAvgSum) * 0.7f; // might be useful for continental??
	//}
	//
	//if (continentalness.type == NoiseType::Cellular) {
	//	continentalness.cellType = ChooseCellular(3, pAudio.midAvgSum * pAudio.highAvgSum); // mids and highs again
	//	continentalness.returnType = ChooseReturn(3, pAudio.midAvgSum * pAudio.highAvgSum); // mids and highs again
	//	continentalness.cellularJitter = 0.7f + (pAudio.midAvgSum * 2.0f); // keep around 1, 

	//	// complicated way to calculate a 30/2477 chance but its calculated strangely so hopefully still unpredictable
	//	if (int((pAudio.highMaxSum * pAudio.midMaxSum * pAudio.bassMaxSum) * (pAudio.highAvgSum * 1000) - (avgBands.highHigh * 1000)) % 2477 <= 30) {
	//		continentalness.cellularJitter = 0.0f + 10 * avgBands.highHigh; // VERY RARE hopefully
	//	}
	//}

	//continentalness.domainWarp = ChooseWarp(3, pAudio.midAvgSum + avgBands.midMid * avgBands.bass); // randomf stuff GO !
	//if (continentalness.domainWarp != WarpType::None) {
	//	continentalness.warpAmplitude = 0.2f + (pAudio.midAvgSum * 0.6f); // keep low .2-.8
	//}

}

void TerrainPreset::print()
{
    std::cout << "////// TERRAIN PRESET CONFIGURATION //////" << std::endl
        << std::endl;
    std::cout << "World Size: " << WORLD_SIZE << std::endl;
    std::cout << "Magic Number: " << magicNumber << std::endl;
    std::cout << "Truncate: " << truncate << std::endl;
    std::cout << "Global Height Scale: " << globalHeightScale << std::endl;
    std::cout << "Exponent: " << exponent << std::endl
        << std::endl;
    std::cout << "/// BASE LAYER ///" << std::endl;
    base.print();
    std::cout << "/// PEAKS AND VALLEYS LAYER ///" << std::endl;
    peaksValleys.print();
    std::cout << "/// CONTINENTALNESS LAYER ///" << std::endl;
    continentalness.print();

}

void NoiseLayerPreset::print()
{
    std::string noiseType;
    std::string warpType;
    std::string fractalType;
    std::string cellularType;
    std::string retType;

    switch (type) {
    case NoiseType::Perlin:
        noiseType = "Perlin";
        break;
    case NoiseType::OpenSimplex2:
        noiseType = "OpenSimplex2";
        break;
    case NoiseType::Value:
        noiseType = "Value";
        break;
    case NoiseType::Cubic:
        noiseType = "Cubic";
        break;
    case NoiseType::Cellular:
        noiseType = "Cellular";
        break;
    }

    switch (fractal) {
    case FractalType::None:
        fractalType = "None";
        break;
    case FractalType::FBm:
        fractalType = "FBm";
        break;
    case FractalType::Ridged:
        fractalType = "Ridged";
        break;
    case FractalType::PingPong:
        fractalType = "PingPong";
        break;
    }

    switch (domainWarp) {
    case WarpType::None:
        warpType = "None";
        break;
    case WarpType::OpenSimplex2:
        warpType = "OpenSimplex2";
        break;
    case WarpType::BasicGrid:
        warpType = "BasicGrid";
        break;
    }

    switch (cellType) {
    case CellularType::None:
        cellularType = "None";
        break;
    case CellularType::Euclidian:
        cellularType = "Euclidian";
        break;
    case CellularType::EuclidianSq:
        cellularType = "EuclidianSq";
        break;
    case CellularType::Manhattan:
        cellularType = "Manhattan";
        break;
    case CellularType::Hybrid:
        cellularType = "Hybrid";
        break;
    }

    switch (returnType) {
    case ReturnType::None:
        retType = "None";
        break;
    case ReturnType::Distance2:
        retType = "Distance2";
        break;
    case ReturnType::Distance2Add:
        retType = "Distance2Add";
        break;
    case ReturnType::Distance:
        retType = "Distance";
        break;
    case ReturnType::Distance2Div:
        retType = "Distance2Div";
        break;
    case ReturnType::Distance2Sub:
        retType = "Distance2Sub";
        break;
    case ReturnType::Distance2Mul:
        retType = "Distance2Mul";
        break;
    case ReturnType::CellValue:
        retType = "CellValue";
        break;
    }

    std::cout << "Seed: " << seed << std::endl;
    std::cout << "Amplitude: " << amplitude << std::endl;
    std::cout << "Frequency: " << frequency << std::endl;
    std::cout << "Noise Type: " << noiseType << std::endl
        << std::endl;
    std::cout << "Fractal Type: " << fractalType << std::endl;
    std::cout << "Fractal Octaves: " << fractalOctaves << std::endl;
    std::cout << "Lacunarity: " << lacunarity << std::endl;
    std::cout << "Gain: " << gain << std::endl;
    std::cout << "Fractal Weighted Strength: " << fractalWeightedStrength << std::endl
        << std::endl;
    std::cout << "Domain Warp Type: " << warpType << std::endl;
    std::cout << "Warp Amplitude: " << warpAmplitude << std::endl
        << std::endl;
    std::cout << "Cellular Type: " << cellularType << std::endl;
    std::cout << "Cellular Return Type: " << retType << std::endl;
    std::cout << "Cellular Jitter: " << cellularJitter << std::endl
        << std::endl;

}
