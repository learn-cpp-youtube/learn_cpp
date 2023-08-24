#include "SpeedData.h"

SpeedData::SpeedData(std::int32_t fps, const TileData& tileData)
{
    const std::int32_t microsecPerSec = 1000000;
    frameIntervalMicrosec = microsecPerSec / fps;

    timeToMoveHeroAPixelOrthogonallyMicrosec = timeToMoveHeroATileOrthogonallyMicrosec
                                               / tileData.GetTileSizeInPixels();

    timeToMoveHeroAPixelDiagonallyMicrosec = timeToMoveHeroAPixelOrthogonallyMicrosec * 4/3;

    // Apply slow down factor.
    timeToMoveHeroATileOrthogonallyMicrosec  *= slowDownFactor;
    timeToMoveHeroAPixelOrthogonallyMicrosec *= slowDownFactor;
    timeToMoveHeroAPixelDiagonallyMicrosec   *= slowDownFactor;
}