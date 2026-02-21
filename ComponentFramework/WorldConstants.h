#pragma once

constexpr int CHUNK_SIZE = 128;
constexpr float CHUNK_WORLD_SIZE = float(CHUNK_SIZE - 1);

// these need to be able to be changed (should move to world class)
//int WORLD_SIZE = 6; // number of chunks along one axis (world is WORLD_SIZE x WORLD_SIZE chunks) just two for now
//float WORLD_OFFSET = (CHUNK_SIZE * WORLD_SIZE) / 2.0f;