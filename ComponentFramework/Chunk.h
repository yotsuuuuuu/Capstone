#pragma once
#include "CoreStructs.h"
#include "WorldConstants.h"
class Chunk
{
	// the stuff i see has individual vbo/ibo per chunk but here lets 
	// just do one for both and then try and see how this works
	
	IndexedVertexBuffer mesh; 

	Vec2 position; // x,z position in world space. y comes from heightmap
	// mesh component;

	//chunk size defined in WorldConstants.h  = 64 (8x8)

public:
	//get chunk width/length
	//static constexpr int GetChunkSize() { return CHUNK_SIZE; }

	Chunk(Vec2 pos); // usually includes renderer but lets just do it from the world and pass a renderer or something later
	~Chunk();


	Vec2 getChunkPos() const { return position; }

private:
	//i already have the noise generated so all i need is to sample the noise and create the mesh.
	void GenerateMesh(); // generate the mesh for the chunk. uneeded

	void InitializeBuffers(); // initialize the vbo/ibo for the chunk

};

