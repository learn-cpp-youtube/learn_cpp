#pragma once

#include "TileData.h"
#include "AreaData.h"

// Ignores the blocking flag.
// tileX, tileY are coordinates of the upper left corner of the tile.
bool BasesIntersect(std::int32_t tileX1, std::int32_t tileY1, const TileBaseRect& base1,
                    std::int32_t tileX2, std::int32_t tileY2, const TileBaseRect& base2);

bool IntersectingAgainstForeground(std::int32_t tileX, std::int32_t tileY, const TileBaseRect& base,
                                   const Area& area, const TileData& tileData);