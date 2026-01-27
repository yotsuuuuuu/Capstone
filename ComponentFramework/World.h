#pragma once
//#include "Chunk.h"
//include "Camera.h" // need to adjust for camera actor
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

//const int WORLD_SIZE_X = 64;
//const int WORLD_SIZE_Y = 16;
//const int WORLD_SIZE_Z = 64;

// will need different "seeds" to change the world generation based on the song
// 
// 


//class Renderer;

class World
{
private:

public:
	World();
	~World();

	Chunk* chunk;
	// Camera* camera; (fix for camera actor)
	//bool OnCreate(/*Renderer* renderer*/);
	//void OnDestroy();

};

