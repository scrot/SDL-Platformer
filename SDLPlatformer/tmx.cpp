#include <filesystem>
#include <sstream>

#include "tinyxml2.h"
#include "include/tmx.h"

using namespace tmx;

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
		map->tileWidth = mapDoc->IntAttribute("tileWidth");
		map->tileWidth = mapDoc->IntAttribute("tileHeight");

	}

	for (XMLElement* child = mapDoc->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
	{
		if (strcmp(child->Name(), "tileset") == 0)
		{
			int firstGid = child->IntAttribute("firstgid");

			// Load tileset XML
			XMLDocument tilesetDoc;
			auto sourcePath = mapPath.parent_path().append(child->Attribute("source"));
			tilesetDoc.LoadFile(sourcePath.string().c_str());
		}
	}
}