#include "AreaData.h"
#include "JsonFileHelper.h"
#include <stdexcept>

using error = std::runtime_error;

std::int32_t Area::GetForegroundTileIndex(std::int32_t x, std::int32_t y) const
{
    return foregroundTiles[y][x];
}

std::int32_t& Area::GetForegroundTileIndex(std::int32_t x, std::int32_t y)
{
    return foregroundTiles[y][x];
}

std::int32_t Area::GetBackgroundTileIndex(std::int32_t x, std::int32_t y) const
{
    return backgroundTiles[y][x];
}

std::int32_t& Area::GetBackgroundTileIndex(std::int32_t x, std::int32_t y)
{
    return backgroundTiles[y][x];
}

void AreaData::Init(const TileData& tileData, const json::Object& metadata)
{
    Free();

    // Read the areas info.
    const json::Object& areasObj = GetValue(metadata, "Areas").GetObject();

    // Read the area info.
    for (const auto& [name, data] : areasObj)
    {
        // Map name to area index.
        std::int32_t areaIndex = static_cast<std::int32_t>(areas.size());
        stringToAreaIndex.insert({name, static_cast<std::int32_t>(areas.size())});

        // Add new area to areas array.
        areas.emplace_back(Area{});
        Area& area = areas.back();

        // Fill in dimension info.
        const json::Object& dataObj = data.GetObject();
        area.widthInTiles  = static_cast<std::int32_t>(
                                GetValue(dataObj, "WidthInTiles").GetInteger());
        area.heightInTiles = static_cast<std::int32_t>(
                                GetValue(dataObj, "HeightInTiles").GetInteger());

        // Resize tile arrays.
        area.backgroundTiles.resize(area.heightInTiles);
        for (std::vector<std::int32_t>& v : area.backgroundTiles)
            v.resize(area.widthInTiles, TileData::NotFound);

        area.foregroundTiles.resize(area.heightInTiles);
        for (std::vector<std::int32_t>& v : area.foregroundTiles)
            v.resize(area.widthInTiles, TileData::NotFound);
        
        // Create character to tile index map.
        std::map<char, std::int32_t> charToTile;

        const json::Object& mappingObj = GetValue(dataObj, "TileMapping").GetObject();
        for (const auto& [tileChar, tileId] : mappingObj)
        {
            if (tileChar.size() != 1)
                throw error("Expecting a single character for tile mapping.");

            std::int32_t tileIndex = tileData.GetTileIndex(tileId.GetString());
            charToTile.insert({tileChar[0], tileIndex});
        }

        // Read background tiles.
        const json::Array& backgroundTilesArray = GetValue(dataObj, "Background").GetArray();
        if (backgroundTilesArray.size() != area.heightInTiles)
            throw error("Tile array height doesn't match HeightInTiles field.");

        for (std::int32_t y=0; y<area.heightInTiles; ++y)
        {
            std::string row = backgroundTilesArray[y].GetString();

            if (row.size() != area.widthInTiles)
                throw error("Tile array width doesn't match WidthInTiles field.");

            for (std::int32_t x=0; x<area.widthInTiles; ++x)
            {
                auto it = charToTile.find(row[x]);
                if (it != charToTile.end())
                    area.backgroundTiles[y][x] = it->second;
            }
        }

        // Read foreground tiles.
        const json::Array& foregroundTilesArray = GetValue(dataObj, "Foreground").GetArray();
        if (foregroundTilesArray.size() != area.heightInTiles)
            throw error("Tile array height doesn't match HeightInTiles field.");

        for (std::int32_t y=0; y<area.heightInTiles; ++y)
        {
            std::string row = foregroundTilesArray[y].GetString();

            if (row.size() != area.widthInTiles)
            {
             
                throw error("Tile array width doesn't match WidthInTiles field.");
            }

            for (std::int32_t x=0; x<area.widthInTiles; ++x)
            {
                auto it = charToTile.find(row[x]);
                if (it != charToTile.end())
                    area.foregroundTiles[y][x] = it->second;
            }
        }
    }
}

void AreaData::Free()
{
    areas.clear();
    stringToAreaIndex.clear();
}

std::int32_t AreaData::GetAreaIndex(const std::string& id) const
{
    auto it = stringToAreaIndex.find(id);
    return (it == stringToAreaIndex.end()) ? NotFound : it->second;
}

Area& AreaData::GetArea(std::int32_t areaIndex)
{
    return areas[areaIndex];
}

const Area& AreaData::GetArea(std::int32_t areaIndex) const
{
    return areas[areaIndex];
}