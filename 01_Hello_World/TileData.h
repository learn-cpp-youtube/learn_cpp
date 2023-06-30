#pragma once

#include "MediaInterface/MediaInterface.h"
#include "Utilities/Json.h"
#include <concepts>
#include <cstdint>
#include <map>
#include <vector>

struct TileBaseRect
{
    // Only rectanglular bases are supported.

    // Bottom left of base rect is given by (leftX, bottomY) in pixel coordinates.
    // (0,0) is the top left pixel in the tile,
    // (tileSize-1,tileSize-1) is the bottom right pixel of the tile.
    std::int32_t leftX   = 0;
    std::int32_t bottomY = 0;

    // Width and height in pixels.
    std::int32_t width  = 0;
    std::int32_t height = 0;

    // If blocking is true, then the base rectangle is not passable by the hero character.
    bool blocking = false;
};

class TileData
{
public:
    static const std::int32_t NotFound = -1;

    TileData() {}
    ~TileData() { Free(); }

    void Init(mi::MediaInterface& mi, const json::Object& metadata);
    void Free();

    std::int32_t GetTileSizeInPixels() const { return tileSizeInPixels; }

    // Returns TileData::NotFound or the index of the tile.
    std::int32_t GetTileIndex(const std::string& id) const;

    const TileBaseRect& GetTileBaseRect(std::int32_t tileIndex) const
    { return tiles[tileIndex].base; } 

    void DrawTile(mi::ImageHandle& destImg, float destX, float destY,
                  std::int32_t srcTileIndex,
                  float alpha = 255, std::int32_t maskTileIndex = NotFound) const;

    template<std::convertible_to<float> DX,
             std::convertible_to<float> DY,
             std::convertible_to<float> A  = float>
    void DrawTile(mi::ImageHandle& destImg, DX destX, DY destY,
                  std::int32_t srcTileIndex,
                  A alpha = 255, std::int32_t maskTileIndex = NotFound) const
    {
        const TileData& temp = *this; // Workaround for compiler bug.
        temp.                         // Workaround for compiler bug.
        DrawTile(destImg,
                 static_cast<float>(destX),
                 static_cast<float>(destY),
                 srcTileIndex,
                 static_cast<float>(alpha),
                 maskTileIndex);
    }

private:
    struct Tile
    {
        std::int32_t tilesetIndex; // Which tileset image does the tile lie in.
        std::int32_t x;
        std::int32_t y;
        TileBaseRect base;
    };

    void FillInBaseData(std::map<std::string, TileBaseRect>& baseTiles);

    std::int32_t tileSizeInPixels = 0;
    std::vector<mi::ImageHandle> tilesets;
    std::vector<Tile> tiles;
    std::map<std::string, std::int32_t> stringToTileIndex;
};
