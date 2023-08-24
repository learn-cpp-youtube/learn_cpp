#include "CollisionDetection.h"

namespace
{

ObjectsIntersecting IntersectingAgainstForegroundHelper(std::int32_t tileX,
                                                        std::int32_t tileY,
                                                        const TileBaseRect& base,
                                                        std::int32_t excludeObject,
                                                        const Area& area,
                                                        const TileData& tileData,
                                                        bool returnAfterFirstIntersection)
{
    // TODO: make this more efficient (i.e. don't check for intersection with everything instead
    // create a cached smaller list of potentials).

    ObjectsIntersecting output;
    const auto& objects = area.foreground.objects;

    for (std::int32_t index=0; index<objects.size(); ++index)
    {
        const auto& obj = objects[index];
        if (obj.tile == TileData::NotFound)
            continue; // No tile.

        const TileBaseRect& baseForegroundTile = tileData.GetTileBaseRect(obj.tile);
        if (!baseForegroundTile.blocking)
            continue;

        if (!BasesIntersect(tileX, tileY, base, obj.x, obj.y, baseForegroundTile))
            continue;

        if (index == excludeObject)
            continue;

        output.objectIndices[output.numberFound] = index;
        output.numberFound += 1;

        if (returnAfterFirstIntersection || output.numberFound == output.objectIndices.size())
            break;
    }

    return output;
}

} // End of anonymous namespace.

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
                                   std::int32_t excludeObject,
                                   const Area& area,
                                   const TileData& tileData)
{
    ObjectsIntersecting listOfIntersecting = IntersectingAgainstForegroundHelper(tileX, tileY, base,
                                                excludeObject, area, tileData, true);
    return (listOfIntersecting.numberFound != 0);
}

ObjectsIntersecting ListOfIntersectingForegroundObjects(std::int32_t tileX,
                                                        std::int32_t tileY,
                                                        const TileBaseRect& base,
                                                        std::int32_t excludeObject,
                                                        const Area& area,
                                                        const TileData& tileData)
{
    return IntersectingAgainstForegroundHelper(tileX, tileY, base,
                                               excludeObject, area, tileData, false);
}
