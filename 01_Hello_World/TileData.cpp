#include "TileData.h"
#include "JsonFileHelper.h"
#include <stdexcept>

using error = std::runtime_error;

namespace
{

mi::Color ConvertToColor(const std::string& str)
{
    if (str.size() < 3)
        throw error("Color string must begin with the prefix '0x' and have at least one digit.");

    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X'))
        throw error("Color string must begin with the prefix '0x'.");

    std::uint32_t value = 0;
    std::size_t index = 2;
    while(index < str.size())
    {
        char c = str[index];
        ++index;

        value <<= 4;
        if ('0' <= c  && c <= '9')
            value |= c - '0';
        else
        if ('a' <= c && c <= 'f')
            value |= c - 'a' + 10;
        else
        if ('A' <= c && c <= 'F')
            value |= c - 'A' + 10;
        else
            throw error("Non-hex character encountered when converting color.");

        if (value > 0xffffff)
            throw error("Colour out of range must be between 0 and 0xFFFFFF.");
    }

    return mi::Color((value << 8) | 0xff); 
}

} // End of anonymous namespace.

void TileData::Init(mi::MediaInterface& mi, const json::Object& metadata)
{
    Free();

    // Read the general info.
    const json::Object& generalObj = GetValue(metadata, "General").GetObject();

    tileSizeInPixels = static_cast<std::int32_t>(GetValue(generalObj, "TileSize").GetInteger());
    mi::Color keyColor = ConvertToColor(GetValue(generalObj, "KeyColor").GetString());

    // Read the tilesets info.
    Tile tile{};
    const json::Object& tilesetsObj = GetValue(metadata, "Tilesets").GetObject();
    for (const auto& [name, data] : tilesetsObj)
    {
        tile.tilesetIndex = static_cast<std::int32_t>(tilesets.size());

        // Read filepath and create image.
        const std::string& filepath = GetValue(data.GetObject(), "Filepath").GetString();
        
        mi::ImageHandle tempImg = mi.CreateImage(filepath);
        tilesets.push_back(mi.CreateImage(tempImg, keyColor));

        // Read the tile data.
        const json::Object& tileDataObj = GetValue(data.GetObject(), "TileData").GetObject();
        for (const auto& [tileName, tileData] : tileDataObj)
        {
            std::int32_t tileIndex = static_cast<std::int32_t>(tiles.size());
            const json::Array& posArray = GetValue(tileData.GetObject(), "Pos").GetArray();

            if (posArray.size() != 2)
                throw error("Expected \"Pos\" to be an array of size 2.");

            tile.x = static_cast<std::int32_t>(posArray[0].GetInteger());
            tile.y = static_cast<std::int32_t>(posArray[1].GetInteger());

            tiles.push_back(tile);
            stringToTileIndex.insert({name+"/"+tileName, tileIndex});
        }
    }
}

void TileData::Free()
{
    tileSizeInPixels = 0;

    tilesets.clear();
    tiles.clear();
    stringToTileIndex.clear();
}

std::int32_t TileData::GetTileIndex(const std::string& id) const
{
    auto it = stringToTileIndex.find(id);
    return (it == stringToTileIndex.end()) ? NotFound : it->second;
}

void TileData::DrawTile(mi::ImageHandle& destImg,
                        float destX,
                        float destY,
                        std::int32_t srcTileIndex,
                        float alpha,
                        std::int32_t maskTileIndex) const
{
    const Tile& srcTile = tiles[srcTileIndex];

    if (maskTileIndex != NotFound)
    {
        const Tile& maskTile = tiles[maskTileIndex];
        destImg.DrawImage(destX, destY,
                          tilesets[srcTile.tilesetIndex],
                          srcTile.x, srcTile.y, tileSizeInPixels, tileSizeInPixels,
                          alpha,
                          &tilesets[maskTile.tilesetIndex], maskTile.x, maskTile.y);
    }
    else
    {
        destImg.DrawImage(destX, destY,
                          tilesets[srcTile.tilesetIndex],
                          srcTile.x, srcTile.y, tileSizeInPixels, tileSizeInPixels,
                          alpha);
    }
}