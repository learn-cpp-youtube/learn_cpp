#include "CollisionDetection.h"

bool BasesIntersect(std::int32_t tileX1, std::int32_t tileY1, const TileBaseRect& base1,
                    std::int32_t tileX2, std::int32_t tileY2, const TileBaseRect& base2)
{
    // Check if base 1 is entirely to the left of base 2.
    if (tileX1 + base1.leftX + base1.width - 1 < tileX2 + base2.leftX)
        return false;

    // Check if base 2 is entirely to the left of base 1.
    if (tileX2 + base2.leftX + base2.width - 1 < tileX1 + base1.leftX)
        return false;

    // Check if base 1 is entirely above base 2.
    if (tileY1 + base1.bottomY < tileY2 + base2.bottomY - base2.height + 1)
        return false;

    // Check if base 2 is entirely above base 1.
    if (tileY2 + base2.bottomY < tileY1 + base1.bottomY - base1.height + 1)
        return false;

    return true; // Intersecting.
}

bool IntersectingAgainstForeground(std::int32_t tileX,
                                   std::int32_t tileY,
                                   const TileBaseRect& base,
                                   const Area& area,
                                   const TileData& tileData)
{
    // TODO: make this more efficient (i.e. don't check for intersection with everything instead
    // create a cached smaller list of potentials).

    const std::int32_t tileSize = tileData.GetTileSizeInPixels();

    for (std::int32_t yIndexPos=0; yIndexPos<area.heightInTiles; ++yIndexPos)
    for (std::int32_t xIndexPos=0; xIndexPos<area.widthInTiles;  ++xIndexPos)
    {
        std::int32_t tileIndex = area.GetForegroundTileIndex(xIndexPos, yIndexPos);
        
        if (tileIndex == TileData::NotFound)
            continue; // No tile.

        const TileBaseRect& baseForegroundTile = tileData.GetTileBaseRect(tileIndex);
        if (!base.blocking)
            continue;

        if (BasesIntersect(tileX, tileY, base,
                           xIndexPos*tileSize, yIndexPos*tileSize, baseForegroundTile))
        {
            return true;
        }
    }

    return false;
}