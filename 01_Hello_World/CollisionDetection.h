#pragma once

#include "TileData.h"
#include "AreaData.h"
#include <array>

// Avoid dynamic memory allocation by using a fixed size array.
struct ObjectsIntersecting
{
    static const std::int32_t maxObjects = 4; // Small arbitrary value which is good enough.
    std::int32_t numberFound = 0;
    std::array<std::int32_t, maxObjects> objectIndices{};
};

// Ignores the blocking flag.
// tileX, tileY are coordinates of the upper left corner of the tile.
bool BasesIntersect(std::int32_t tileX1, std::int32_t tileY1, const TileBaseRect& base1,
                    std::int32_t tileX2, std::int32_t tileY2, const TileBaseRect& base2);

bool IntersectingAgainstForeground(std::int32_t tileX, std::int32_t tileY, const TileBaseRect& base,
                                   const Area& area, const TileData& tileData);

ObjectsIntersecting ListOfIntersectingForegroundObjects(std::int32_t tileX,
                                                        std::int32_t tileY,
                                                        const TileBaseRect& base,
                                                        const Area& area,
                                                        const TileData& tileData);