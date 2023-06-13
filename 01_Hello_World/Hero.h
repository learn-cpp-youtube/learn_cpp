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

    enum State
    {
        Still,
        Moving
    };

    State        state  = Still;
    Dir          facing = Down;
    std::int32_t x      = 0;
    std::int32_t y      = 0;

    std::int32_t timeToMove = 300; // Milliseconds taken to move between tiles.

    // Keeps track of the last key presses.
    Dir keyPrimaryDir   = None;
    Dir keySecondaryDir = None;

    // Animation timers to animate the position.
    std::int32_t animCurrTime = 0;
    std::int32_t animMaxTime  = 0;
    std::int32_t animEndX     = 0;
    std::int32_t animEndY     = 0;

    // Seperate animation timers to correctly animate the tile.
    std::int32_t animTileCurrTime = 0;
    std::int32_t animTileOffset   = 0;

    std::int32_t tileSizeInPixels = 16;
    std::int32_t fps = 60;

    // The tiles are in the order [Up-Still, Up-Moving, Right-Still, Right Moving,
    // Down-Still, Down-Moving, Left-Still, Left-Moving].
    // Note that this matches the order of the Dir enum. 
    std::array<std::int32_t, 8> tileIndices = {};

    const Area* area = nullptr;
};