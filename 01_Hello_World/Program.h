#pragma once

#include "MediaInterface/MediaInterface.h"
#include "Hero.h"
#include "AreaData.h"
#include "TileData.h"
#include "SpeedData.h"
#include "KeyStates.h"
#include <cmath>
#include <vector>
#include <array>

// Press Escape or Q to quit.
// Press the arrow keys to move.
// Press F1 to toggle showing the keys pressed.
// Press F5 to refresh.
class Program : public mi::Program
{
public:
    explicit Program(mi::MediaInterface* mi);
    void MainLoop() override;

    // Functions that our program uses.
    void HandleInput();
    void Display();

private:
    void Reload();

    // Store data we want to remember and persist between function calls.
    mi::MediaInterface* mi;
    mi::ImageHandle     screen; 

    std::int32_t fps   = 60;
    std::int32_t scale = 3*2;

    std::int32_t backgroundWidthInTiles;
    std::int32_t backgroundHeightInTiles;
    std::int32_t tileSizeInPixels;

    std::int32_t areaIndex;

    AreaData  areaData;
    TileData  tileData;
    SpeedData speedData;
    
    KeyStates keyStates;
    KeyStatesDisplayHelper keyStatesDisplayHelper;
    bool displayKeysPressed = false;

    Hero hero;
};

