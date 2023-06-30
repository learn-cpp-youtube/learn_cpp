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
    // (In the first pass don't read the base data but keep a list of base tiles.)
    Tile tile{};
    std::map<std::string, TileBaseRect> baseTiles;
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

            const json::Object& tileObj = tileData.GetObject();
            const json::Array& posArray = GetValue(tileObj, "Pos").GetArray();

            if (posArray.size() != 2)
                throw error("Expected \"Pos\" to be an array of size 2.");

            tile.x = static_cast<std::int32_t>(posArray[0].GetInteger());
            tile.y = static_cast<std::int32_t>(posArray[1].GetInteger());

            tiles.push_back(tile);
            stringToTileIndex.insert({name+"/"+tileName, tileIndex});
            
            // Check if there is an associated base tile. 
            auto it = tileObj.find("Base");
            if (it != tileObj.end())
                baseTiles[it->second.GetString()] = TileBaseRect{}; // Placeholder.
        }
    }

    FillInBaseData(baseTiles);

    // Second pass, fill in the base rect data for each tile.
    TileBaseRect defaultBase = {0, tileSizeInPixels-1, tileSizeInPixels, tileSizeInPixels, false};
    for (const auto& [name, data] : tilesetsObj)
    {
        // Read the tile data.
        const json::Object& tileDataObj = GetValue(data.GetObject(), "TileData").GetObject();
        for (const auto& [tileName, tileData] : tileDataObj)
        {
            const json::Object& tileObj = tileData.GetObject();
            std::int32_t index = GetTileIndex(name+"/"+tileName);
                        
            // Check if there is an associated base tile. 
            auto it = tileObj.find("Base");
            if (it != tileObj.end())
                tiles[index].base = baseTiles[it->second.GetString()];
            else
                tiles[index].base = defaultBase;
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

void TileData::FillInBaseData(std::map<std::string, TileBaseRect>& baseTiles)
{
    // Create a list of each of the base tiles for each tileset.
    std::map<std::int32_t, std::vector<std::string>> tilesetToTiles;

    for (const auto& [name, base] : baseTiles)
    {
        std::int32_t tileIndex = GetTileIndex(name);
        if (tileIndex == TileData::NotFound)
            throw error("Could not find tile data for \""+name+"\".");

        std::int32_t tilesetIndex = tiles[tileIndex].tilesetIndex;
        tilesetToTiles[tilesetIndex].push_back(name);
    }

    // For each tileset get the pixel data.
    for (const auto& [tilesetIndex, listOfTiles] : tilesetToTiles)
    {
        const mi::ImageHandle& img = tilesets[tilesetIndex];
        std::vector<std::uint8_t> pixelData = img.GetPixelData();
        std::int32_t width = img.GetWidth();

        // For each tile find the base rectangle data by looking at each pixel in the tile.
        for (const std::string& tilename : listOfTiles)
        {
            std::int32_t firstX = -1; // Use -1 to represent unset.
            std::int32_t firstY = -1;
            std::int32_t lastX  = -1;
            std::int32_t lastY  = -1;

            const Tile& tile = tiles[GetTileIndex(tilename)];
            const std::int32_t offsetX = tile.x;
            const std::int32_t offsetY = tile.y;

            bool blocking = true;

            for (std::int32_t y=0; y<tileSizeInPixels; ++y)
            for (std::int32_t x=0; x<tileSizeInPixels; ++x)
            {
                const std::int32_t xInTileSet = x + offsetX;
                const std::int32_t yInTileSet = y + offsetY;
                const std::int32_t index = (yInTileSet*width + xInTileSet) * 4;
                const std::uint8_t r = pixelData[index];
                const std::uint8_t g = pixelData[index+1];
                const std::uint8_t b = pixelData[index+2];

                if (!((r==255 && g==0 && b==0) || (r==0 && g==255 && b==0)))
                    continue; // Not red or green.

                if (g == 255)
                    blocking = false; // A green base represents non-blocking.

                if (firstX == -1)
                {
                    firstX = x;
                    firstY = y;
                }

                lastX = x;
                lastY = y;
            }

            // Fill in the base data.
            TileBaseRect& base = baseTiles[tilename];
            if (firstX == -1)
            {
                base.leftX    = 0;
                base.bottomY  = 0;
                base.width    = 0;
                base.height   = 0;
                base.blocking = false;
            }
            else
            {
                base.leftX    = firstX;
                base.bottomY  = lastY;
                base.width    = lastX - firstX + 1;
                base.height   = lastY - firstY + 1;
                base.blocking = blocking;
            }
        }
    }
}