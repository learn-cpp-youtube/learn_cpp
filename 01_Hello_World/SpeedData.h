#pragma once

#include "TileData.h"
#include <cstdint>

struct SpeedData
{
    static const std::int32_t slowDownFactor = 1; // Useful for debugging.

    SpeedData() = default;
    SpeedData(std::int32_t fps, const TileData& tileData);

    std::int32_t timeToMoveHeroATileOrthogonallyMicrosec = 300000;

    // Values filled in by constructor.
    std::int32_t frameIntervalMicrosec = 0;
    std::int32_t timeToMoveHeroAPixelOrthogonallyMicrosec = 0; 
    std::int32_t timeToMoveHeroAPixelDiagonallyMicrosec   = 0;
};