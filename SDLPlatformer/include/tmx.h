#pragma once

#include <vector>
#include <string>
#include <memory>
#include <variant>

// Types and loader for Tiled (https://www.mapeditor.org/) .tmx map files and
// their referenced .tsx tileset files.
//
// NOTE: This loader is a work in progress. It is not currently wired up to
// the game loop -- SDLPlatformer.cpp's createTiles() still builds the level
// from hardcoded arrays, and assets/maps/*.tmx are not loaded at runtime.
namespace tmx
{
	// A tile layer: a grid of tile IDs, stored flattened in row-major order.
	struct Layer
	{
		int id;
		std::string name;
		std::vector<int> data;	// Flattened tile-ID grid; length should be map width * height
	};

	// A single object placed via a Tiled object layer (e.g. spawn points, triggers).
	struct LayerObject
	{
		int id;
		std::string name;
		std::string type;
		float x;
		float y;
	};

	// A Tiled object group layer: a named collection of freely-placed objects.
	struct ObjectGroup
	{
		int id;
		std::string name;
		std::vector<LayerObject> objects;
	};

	// The source image backing a single tile in a tileset.
	struct Image
	{
		int id;
		std::string source;
		int width;
		int height;
	};

	// A single tile definition within a TileSet.
	struct Tile
	{
		int id;
		Image image;
	};

	// A Tiled tileset (.tsx), describing the tile images used by one or more layers.
	struct TileSet
	{
		int count;			// Total number of tiles in the tileset
		int tileWidth;		// Width of a single tile, in pixels
		int tileHeight;		// Height of a single tile, in pixels
		int columns;		// Number of tile columns in the tileset's source image
		int firstgid;		// Global tile ID that this tileset's tile 0 maps to within the owning map
		std::vector<Tile> tiles;

	public:
		TileSet(int firstgid, int count, int tileWidth, int tileHeight, int columns)
			: firstgid(firstgid), count(count), tileWidth(tileWidth), tileHeight(tileHeight), columns(columns)
		{
		}
	};

	// A fully parsed Tiled map: its tilesets and its ordered list of tile/object layers.
	struct Map
	{
		int mapWidth;
		int mapHeight;
		int tileWidth;
		int tileHeight;
		std::vector<TileSet> tileSets;
		std::vector<std::variant<Layer, ObjectGroup>> layers;
	};

	/**
	 * @brief Parses a Tiled .tmx map file (and its referenced .tsx tilesets)
	 * into a Map structure.
	 *
	 * @param mapFilePath -- Path to the .tmx file to load. Tileset `source`
	 * paths within it are resolved relative to this file's directory.
	 *
	 * @return The parsed Map.
	 */
	std::unique_ptr<Map> loadMap(const std::string& mapFilePath);


};
