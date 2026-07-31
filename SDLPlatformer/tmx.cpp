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
 * hardcoded arrays.
 *
 * @param mapFilePath -- Path to the .tmx file to load.
 *
 * @return The parsed Map, or nullptr if mapFilePath could not be parsed as a
 * Tiled <map> document.
 */
std::unique_ptr<Map> tmx::loadMap(const std::string& mapFilePath)
{
	using namespace tinyxml2;

	std::filesystem::path mapPath(mapFilePath);

	tmx::Map* map = nullptr;

	XMLDocument doc;
	doc.LoadFile(mapPath.string().c_str());
	XMLElement* mapDoc = doc.FirstChildElement("map");

	// If the file failed to load or isn't a Tiled map, there's nothing to parse.
	if (!mapDoc)
		return nullptr;

	map = new tmx::Map;
	map->mapWidth = mapDoc->IntAttribute("width");
	map->mapHeight = mapDoc->IntAttribute("height");
	map->tileWidth = mapDoc->IntAttribute("tilewidth");
	map->tileHeight = mapDoc->IntAttribute("tileheight");

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
				int columns = ts->IntAttribute("columns");

				tmx::TileSet newTileset(firstgid, count, tileWidth, tileHeight, columns);

				// Walk each <tile> element and pull its <image> child's source/size.
				for (XMLElement* tile = ts->FirstChildElement("tile"); tile != nullptr; tile = tile->NextSiblingElement("tile"))
				{
					Tile newTile;
					newTile.id = tile->IntAttribute("id");

					XMLElement* image = tile->FirstChildElement("image");

					if (image)
					{
						const char* source = image->Attribute("source");

						if (source)
							newTile.image.source = source;

						newTile.image.width = image->IntAttribute("width");
						newTile.image.height = image->IntAttribute("height");
					}

					newTileset.tiles.push_back(newTile);
				}

				map->tileSets.push_back(std::move(newTileset));
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