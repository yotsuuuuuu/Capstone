#pragma once

constexpr int CHUNK_SIZE = 32;
constexpr float CHUNK_WORLD_SIZE = float(CHUNK_SIZE - 1);

constexpr int WORLD_SIZE = 2; // number of chunks along one axis (world is WORLD_SIZE x WORLD_SIZE chunks) just two for now