#pragma once

#include "AreaData.h"
#include "TileData.h"
#include "KeyStates.h"
#include <array>
#include <cstdint>

class Hero
{
public:
    Hero(){}

    Hero(const TileData& tileData, const Area& area, std::int32_t fps);

    std::int32_t GetTileIndex();
    std::int32_t GetXPos() { return x; }
    std::int32_t GetYPos() { return y; }

    void Update(const KeyStates& keyStates);

private: 
    // The enum values are used to index into an array, so it is important the values 0-4 are
    // used.
    enum Dir
    {
        Up    = 0,
        Right = 1,
        Down  = 2,
        Left  = 3,
        None  = 4
    };

    void SetDirFromKeys(const KeyStates& keyStates);
    static void UpdateOffsetsFromDir(std::int32_t& offsetX, std::int32_t& offsetY, Dir dir);
    
    // Returns true if suceeded (and updates x, y and timeLeft).
    // Returns false if failed (no updates are done).
    bool TryMoving(Dir dir1, Dir dir2, std::int32_t& timeLeft, const TileBaseRect& base);

    // Returns None if can't glance.
    Dir DetermineGlanceDir(Dir primaryDir, const TileBaseRect& base);

    enum State
    {
        Still,
        Moving
    };

    State        state  = Still;
    Dir          facing = Down;
    std::int32_t x      = 0;
    std::int32_t y      = 0;

    // Time to move 1 tile distance orthogonally.
    std::int32_t timeToMoveATileOrthogonallyMicrosec  = 300000;
    std::int32_t timeToMoveAPixelOrthogonallyMicrosec = 0; // Filled in by constructor.
    std::int32_t timeToMoveAPixelDiagonallyMicrosec   = 0; // Filled in by constructor.

    std::int32_t maxGlancePixels = 4; // Allow the character to glance by this number of pixels.

    // Keeps track of the last key presses.
    Dir keyPrimaryDir   = None;
    Dir keySecondaryDir = None;

    // Animation timer to ensure smooth movements between frames.
    std::int32_t animMoveTimeOverflowMicrosec = 0;

    // Animation timers to correctly animate the tile.
    std::int32_t animTileTimeMicrosec = 0;
    std::int32_t animTileOffset       = 0; // Flips between 0 and 1

    std::int32_t frameIntervalMicrosec = 0; // Filled in by constructor.

    // The tiles are in the order [Up-Still, Up-Moving, Right-Still, Right Moving,
    // Down-Still, Down-Moving, Left-Still, Left-Moving].
    // Note that this matches the order of the Dir enum. 
    std::array<std::int32_t, 8> tileIndices = {};

    const Area* area = nullptr;
    const TileData* tileData = nullptr;
};