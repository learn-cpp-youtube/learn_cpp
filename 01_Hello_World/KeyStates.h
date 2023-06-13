#pragma once

#include "MediaInterface/MediaInterface.h"
#include "TileData.h"

struct KeyStates
{
    bool upPressed    = false;
    bool downPressed  = false;
    bool leftPressed  = false;
    bool rightPressed = false;
};

class KeyStatesDisplayHelper
{
public:
    KeyStatesDisplayHelper(){}
    KeyStatesDisplayHelper(mi::MediaInterface& mi, const TileData& tileData);
    mi::ImageHandle& GetImage(const KeyStates& keyStates);

private:
    mi::ImageHandle img;
    const TileData* tileData = nullptr;

    // 0 = unpushed, 1 = pushed. (Maps to the integer conversion of bool 0 = false, 1 = true.)
    std::int32_t upTiles[2]    = {};
    std::int32_t downTiles[2]  = {};
    std::int32_t leftTiles[2]  = {};
    std::int32_t rightTiles[2] = {};

    std::int32_t tileSizeInPixels = 0;
    std::int32_t spacingBetweenTilesInPixels = 0;
};