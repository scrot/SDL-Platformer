// SDLPlatformer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

// TODO: Reference additional headers your program requires here.

// Global constants

// Index into GameState::layers for static level geometry (ground, panels, etc.)
const size_t LAYER_IDX_LEVEL = 0;
// Index into GameState::layers for dynamic characters (player, enemies)
const size_t LAYER_IDX_CHARACTERS = 1;
// Number of rows in the hardcoded level tile grids used by createTiles()
const int MAP_ROWS = 5;
// Number of columns in the hardcoded level tile grids used by createTiles()
const int MAP_COLS = 50;
// Width/height in pixels of a single square tile and sprite animation frame
const int TILE_SIZE = 32;
