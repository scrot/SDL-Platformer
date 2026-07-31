#include <filesystem>
#include <sstream>

#include "tinyxml2.h"
#include "include/tmx.h"

using namespace tmx;

/**
 * @brief Parses a Tiled .tmx map file into a tmx::Map (see tmx.h).
 *
 * Walks the top-level children of the <map> element and, per Tiled's XML
 * schema, dispatches on tag name: <tileset> references are resolved to a
 * sibling .tsx file and parsed into a TileSet; <layer> elements have their
 * comma-separated tile ID grid parsed into a Layer; <objectgroup> elements
 * have their child <object> elements parsed into an ObjectGroup.
 *
 * NOTE: This function is not yet called anywhere in the game loop -- level
 * geometry is still built by SDLPlatformer.cpp's createTiles() from
 * hardcoded arrays. It also currently has several known bugs that mean it
 * will not correctly parse a real .tmx/.tsx file yet (flagged inline below):
 * a couple of misspelled/mis-cased XML attribute names, a map height/width
 * field overwrite, and a tile-image loop that references its own
 * uninitialized loop variable.
 *
 * @param mapFilePath -- Path to the .tmx file to load.
 *
 * @return The parsed Map, or a Map left partially/never initialized if
 * mapFilePath could not be parsed as a <map> document (see BUG note below).
 */
std::unique_ptr<Map> tmx::loadMap(const std::string& mapFilePath)
{
	using namespace tinyxml2;

	std::filesystem::path mapPath(mapFilePath);

	tmx::Map* map = nullptr;

	XMLDocument doc;
	doc.LoadFile(mapPath.string().c_str());
	XMLElement* mapDoc = doc.FirstChildElement("map");

	if (mapDoc)
	{
		map = new tmx::Map;
		map->mapWidth = mapDoc->IntAttribute("width");
		map->mapHeight = mapDoc->IntAttribute("height");
		// BUG: Tiled's actual XML attributes are "tilewidth"/"tileheight" (lowercase),
		// not "tileWidth"/"tileHeight", so both IntAttribute() calls below return 0.
		// The second line also assigns into tileWidth again instead of tileHeight,
		// so map->tileHeight is never set at all.
		map->tileWidth = mapDoc->IntAttribute("tileWidth");
		map->tileWidth = mapDoc->IntAttribute("tileHeight");

	}

	// BUG: if mapDoc is null (e.g. the file failed to load or isn't a Tiled
	// map), this dereferences a null pointer instead of returning early.
	for (XMLElement* child = mapDoc->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
	{
		if (strcmp(child->Name(), "tileset") == 0)
		{
			int firstgid = child->IntAttribute("firstgid");

			// Load tileset XML
			XMLDocument tilesetDoc;
			auto sourcePath = mapPath.parent_path().append(child->Attribute("source"));
			tilesetDoc.LoadFile(sourcePath.string().c_str());

			XMLElement* ts = tilesetDoc.FirstChildElement("tileset");

			if (ts)
			{
				int tileWidth = ts->IntAttribute("tilewidth");
				int tileHeight = ts->IntAttribute("tileheight");
				int count = ts->IntAttribute("tilecount");
				// BUG: Tiled's tileset column-count attribute is "columns", not
				// "tilecolumns" -- this always reads back 0.
				int columns = ts->IntAttribute("tilecolumns");

				tmx::TileSet newTileset(firstgid, count, tileWidth, tileHeight, columns);

				// BUG: `tile` is read (via tile->FirstChildElement) before it has been
				// initialized by its own loop -- this is undefined behavior, not a walk
				// over ts's child <tile> elements. This loop needs to start from
				// ts->FirstChildElement("tile") instead.
				for (XMLElement* tile = tile->FirstChildElement("image"); tile != nullptr; tile = tile->NextSiblingElement("image"))
				{
					Tile newTile;
					newTile.id = tile->IntAttribute("id");

					XMLElement* image = tile->FirstChildElement("image");

					if (image)
					{
						// BUG: Image::source is a std::string (the "source" attribute,
						// e.g. "../tiles/brick.png"), but this reads a nonexistent
						// "image" attribute as an int and assigns it to a string.
						newTile.image.source = image->IntAttribute("image");
						newTile.image.width = image->IntAttribute("width");
						newTile.image.height = image->IntAttribute("height");
					}

					newTileset.tiles.push_back(newTile);
				}

				// NOTE: newTileset is never added to map->tileSets here, so parsed
				// tilesets are currently discarded.
			}
		}
		else
			// <layer>: a tile grid layer. Its <data> child holds a comma-separated
			// list of tile IDs (CSV encoding), parsed here one integer at a time.
			if (strcmp(child->Name(), "layer") == 0)
			{
				tmx::Layer layer;
				layer.name = child->Attribute("name");
				layer.id = child->IntAttribute("id");
				layer.data.reserve(map->mapWidth * map->mapHeight);

				XMLElement* data = child->FirstChildElement("data");
				std::stringstream dataStream(data->GetText());

				for (int i; dataStream >> i;)
				{
					layer.data.push_back(i);

					if (dataStream.peek() == ',')
						dataStream.ignore();
				}

				map->layers.push_back(std::move(layer));
			}
			else
				// <objectgroup>: a layer of freely-placed <object> elements
				// (e.g. spawn points/triggers) rather than a tile grid.
				if (strcmp(child->Name(), "objectgroup") == 0)
				{
					tmx::ObjectGroup layer;
					layer.name = child->Attribute("name");
					layer.id = child->IntAttribute("id");

					for (XMLElement* elem = child->FirstChildElement("object"); elem != nullptr; elem = elem->NextSiblingElement())
					{
						tmx::LayerObject obj;

						obj.id = elem->IntAttribute("id");
						obj.x = elem->FloatAttribute("x");
						obj.y = elem->FloatAttribute("y");

						const char* attrName = elem->Attribute("name");

						if (attrName)
							obj.name = attrName;

						const char* attrType = elem->Attribute("type");

						if (attrType)
							obj.type = attrType;

						layer.objects.push_back(obj);
					}

					map->layers.push_back(std::move(layer));
				}
	}

	// Ownership of `map` (raw new'd above) transfers to the returned unique_ptr.
	return std::unique_ptr<tmx::Map>(map);
}