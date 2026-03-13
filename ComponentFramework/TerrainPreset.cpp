#include "TerrainPreset.h"

void TerrainPreset::CreateFromAudio(std::vector<AudioBands> ab)
{
	pAudio = GetLayerValuesFromAudio(ab);

	CreateBase();
	//CreatePeaksValleys();
	//CreateErosion();
	//CreateContinentalness();
	exponent = 1.0f + (int(pAudio.bassMaxSum * pAudio.midMaxSum - pAudio.bassAvgSum * pAudio.highMaxSum) % 2) * pAudio.bassAvgSum;
	std::cout << exponent << std::endl;
	int why = 0;
	// TODO: (andres) exponents, 
	// TODO: (andres) global height scale, 
	// TODO: (andres) concatenation (need more modifiers ex. concat can be to certain fractions instead of whole)
	// TODO: (andres) continentallness heights
	// TODO: (andres) randomize actor placement (lights, etc) ( x % worldsize, then x % chunksize)


	//// CONTINENTALNESS
	//continentalness.type = NoiseType::Cellular; // cellular or value cubic seem to be good options
	//continentalness.seed = 3847598; // idk hash the song
	//continentalness.frequency = 0.005f; // for continentalness keep it low. 0.001-0.01
	//continentalness.amplitude = 0.2f; // maybe even 0 since were really jsut using it to determine height ranges. maybe 0.2
	//continentalness.fractal = FractalType::None; // anything but ridged
	//continentalness.fractalOctaves = 1; // if using fractal, 3-5 octaves is good for continentalness (influenced by ??)
	//continentalness.fractalWeightedStrength = 0.0f; // not needed for continentalness
	//continentalness.gain = 0.005f; // keep very very very small 
	//continentalness.cellType = CellularType::Euclidian; // try not to use hybrid or manhattan
	//continentalness.returnType = ReturnType::Distance2Sub; // anything works here jsut keep in mind cell value will be sharp changes
	//continentalness.cellularJitter = 1.0f; // keep around 1, 
	//continentalness.domainWarp = WarpType::None; // not needed for continentalness
	

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
	int numNoise = 5;
	int modifier = 0;
	switch (layer) {
		case 0: // base layer
			numNoise = 2; // all noise types available for base layer since it has the biggest impact on overall terrain shape
			break;
		case 1: // peaks and valleys
		case 2: // erosion
			numNoise = 4; // only 4 noise types for these layers since we want to avoid cellular
			break;
		case 3: // continentalness
			numNoise = 3;
			modifier = 2; // value, cubic, and cellular.
			// [x] [x] [x] [ ] [ ]	// 3 noises available
			// [ ] [ ] [x] [x] [x]	// +2 to get the last 3 noise types
			break;
	}
	
	// result is 0 - numNoise-1;
	int percent = std::round(value * float(numNoise)); 

	// if the final result is greater than the amount of available then just set it to the overflow
	int result = (percent % numNoise) + modifier;
	result = result > numNoise ? (result - numNoise) + modifier : (percent % numNoise) + modifier;

	// ex. [x] [x] [x] [ ] [ ]	// 3 noises available
	//     [ ] [ ] [x] [x] [x]	// +2 to get the last 3 noise types
	// numNoise = 3, modifier = 2 value = 0.5
	// percent = 1.5 -> round to 2
	// result = (2 % 3) + 2 = 4 -> since 4 > numNoise(3) result = 4 - 3 = 1 + modifier(2) = 3 which is the last noise type available
	
	return static_cast<NoiseType>(result); 

}

FractalType TerrainPreset::ChooseFractal(int layer, float value)
{
	int numFractal = 4;
	int modifier = 0;
	// setting value to 0 means the first option will get chosen

	switch (layer) {
		case 0: // base layer
			numFractal = 3; // just any but ridged? looks good in preset 2
			break;
		case 1: // peaks and valleys
			numFractal = 3; // any
			modifier = 1; // FBm, PingPong, Ridged
				// [x] [x] [x] [ ]	// 3 fractal types available
				// [ ] [x] [x] [x]	// +1 to get the last 3 fractal types
			break;

		case 2: // erosion
			// not sure yet rn just using continentalness values
		case 3: // continentalness
			numFractal = 4; //anything
			modifier = 0; 
			break;
	}

	// result is 0 - numFractal-1;
	int percent = std::round(value * float(numFractal));

	// if the final result is greater than the amount of available then just set it to the overflow
	int result = (percent % numFractal) + modifier;
	result = result > numFractal ? (result - numFractal) + modifier : (percent % numFractal) + modifier;
	return static_cast<FractalType>(result);
}

WarpType TerrainPreset::ChooseWarp(int layer, float value)
{
	int numWarp = 3;
	int modifier = 0;
	// setting value to 0 means the first option will get chosen

	switch (layer) {
	case 0: // base layer
		numWarp = 3; // any
		break;
	case 1: // peaks and valleys
		numWarp = 2; // no grid
		break;

	case 2: // erosion
		// not sure yet rn just using continentalness values
	case 3: // continentalness
		numWarp = 2; // has to warp
		modifier = 1; // OpenSimplex2 or BasicGrid
				// [x] [x] [ ]	// 2 warp types available
				// [ ] [x] [x]	// +1 to get the last 2 warp types
		break;
	}

	// result is 0 - numWarp-1;
	int percent = std::round(value * float(numWarp));

	int result = (percent % numWarp) + modifier;
	result = result > numWarp ? (result - numWarp) + modifier : (percent % numWarp) + modifier;
	return static_cast<WarpType>(result);
}

CellularType TerrainPreset::ChooseCellular(int layer, float value)
{

	int numCellular = 5;
	int modifier = 0;

	switch (layer) {
	case 0: // base layer
		value = 0; // not needed
		break;
	case 1: // peaks and valleys
		numCellular = 3; // no hybrid
		modifier = 1; // euclidian, euclidian sq, manhattan
			// [x] [x] [x] [ ] [ ]	// 3 cellular types available
			// [ ] [x] [x] [x] [ ]	// +1 to get the middle 3 cellular types
		break;

	case 2: // erosion
		// not sure yet rn just using continentalness values
	case 3: // continentalness
		numCellular = 4; // 
		modifier = 1; // 
		break;
	}

	int percent = std::round(value * float(numCellular));

	int result = (percent % numCellular) + modifier;
	result = result > numCellular ? (result - numCellular) + modifier : (percent % numCellular) + modifier;
	return static_cast<CellularType>(result);

}

ReturnType TerrainPreset::ChooseReturn(int layer, float value)
{
	int numReturn = 7;
	int modifier = 0;

	switch (layer) {
	case 0: // base layer
		value = 0; // not needed
		break;
	case 1: // peaks and valleys
		numReturn = 3; // 
		modifier = 2; // distance2, distance2add, distance2
			// [x] [x] [x] [ ] [ ] [ ] [ ]	// 3 return types available
			// [ ] [x] [x] [x] [ ] [ ] [ ]	// +2 to get the middle 3 return types
		break;

	case 2: // erosion
		// not sure yet rn just using continentalness values
	case 3: // continentalness
		numReturn = 4; // 
		modifier = 1; // all basically
			// [x] [x] [x] [x] [x] [x] [x] [ ]	// 4 return types available
			// [ ] [x] [x] [x] [x] [x] [x] [x]	// +1 to get the last 4 return types
		break;
	}

	int percent = std::round(value * float(numReturn));

	int result = (percent % numReturn) + modifier;
	result = result > numReturn ? (result - numReturn) + modifier : (percent % numReturn) + modifier;
	return static_cast<ReturnType>(result);
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
	base.frequency = 0.005f + (avgBands.sub * 0.03f); // for base keep it very very low. 0.001-0.01
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
	peaksValleys.frequency = 0.05f + (pAudio.highAvgSum * 0.1f); // for peaks and valleys we want a bit more frequency than base to add some variation, but not too much that it becomes noisy
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
	AudioBands avgBands = pAudio.avgBands;
	continentalness.type = ChooseNoise(3, (pAudio.midAvgSum * pAudio.highAvgSum )/3.0f); // mids and highs since they can add some variation without completely changing the overall shape of the terrain
	continentalness.seed = pAudio.seed * (pAudio.midAvgSum + pAudio.highAvgSum + 1); // vary seed slightly for each layer
	continentalness.frequency = 0.08f * ((avgBands.midMid + avgBands.lowMid)); // almost lower than base
	continentalness.amplitude = 0.2f + (pAudio.midAvgSum * 0.33f); 
	
	//continentalness.fractal = ChooseFractal(3,pAudio.bassMaxSum * avgBands.midHigh * 0.01f);
	if (continentalness.fractal != FractalType::None) {
		continentalness.fractalOctaves = 1 + std::round((pAudio.midAvgSum) * 1.2f); // barely any octaves 
		continentalness.gain = 0.005f + (pAudio.midAvgSum * 0.05f); // keep very very very small 
		continentalness.lacunarity = 1.7f + (pAudio.midAvgSum * 0.6f);
		continentalness.fractalWeightedStrength = (pAudio.midAvgSum) * 0.7f; // might be useful for continental??
	}
	
	if (continentalness.type == NoiseType::Cellular) {
		continentalness.cellType = ChooseCellular(3, pAudio.midAvgSum * pAudio.highAvgSum); // mids and highs again
		continentalness.returnType = ChooseReturn(3, pAudio.midAvgSum * pAudio.highAvgSum); // mids and highs again
		continentalness.cellularJitter = 0.7f + (pAudio.midAvgSum * 2.0f); // keep around 1, 

		// complicated way to calculate a 10/2477 chance but its calculated strangely so hopefully still unpredictable
		if (int((pAudio.highMaxSum * pAudio.midMaxSum * pAudio.bassMaxSum) * (pAudio.highAvgSum * 1000) - (avgBands.highHigh * 1000)) % 2477 <= 10) {
			continentalness.cellularJitter = 0.0f + 10 * avgBands.highHigh; // VERY RARE hopefully
		}
	}

	continentalness.domainWarp = ChooseWarp(3, pAudio.midAvgSum + avgBands.midMid * avgBands.bass); // randomf stuff GO !
	if (continentalness.domainWarp != WarpType::None) {
		continentalness.warpAmplitude = 0.2f + (pAudio.midAvgSum * 0.6f); // keep low .2-.8
	}

}



