#include "Hero.h"
#include <algorithm>
#include <array>
#include <vector>

Hero::Hero(const TileData& tileData, const Area& area, std::int32_t fps)
{
    this->area             = &area;
    this->tileSizeInPixels = tileData.GetTileSizeInPixels();
    this->fps              = fps;

    tileIndices[0] = tileData.GetTileIndex("TestTiles/HeroFacingUpStanding");
    tileIndices[1] = tileData.GetTileIndex("TestTiles/HeroFacingUpMoving");
    tileIndices[2] = tileData.GetTileIndex("TestTiles/HeroFacingRightStanding");
    tileIndices[3] = tileData.GetTileIndex("TestTiles/HeroFacingRightMoving");
    tileIndices[4] = tileData.GetTileIndex("TestTiles/HeroFacingDownStanding");
    tileIndices[5] = tileData.GetTileIndex("TestTiles/HeroFacingDownMoving");
    tileIndices[6] = tileData.GetTileIndex("TestTiles/HeroFacingLeftStanding");
    tileIndices[7] = tileData.GetTileIndex("TestTiles/HeroFacingLeftMoving");
    return;
}

std::int32_t Hero::GetTileIndex()
{
    std::int32_t faceOffset = 2 * static_cast<std::int32_t>(facing);

    if (state == Still)
        return tileIndices[faceOffset];
        
    // In the case state == Moving.
    return tileIndices[faceOffset + animTileOffset];
}

void Hero::Update(const KeyStates& keyStates)
{
    // Assume the new key presses were in effect since the last call to update.
    Dir oldPrimaryDir   = keyPrimaryDir;
    Dir oldSecondaryDir = keySecondaryDir;

    SetDirFromKeys(keyStates);

    // Check if the keys being pressed have changed.
    if (keyPrimaryDir != oldPrimaryDir || keySecondaryDir != oldSecondaryDir)
    {
        if (keyPrimaryDir == None)
        {
            state = Still;
            return;
        }
        
        state = Moving;

        // Check if we need to change the direction the character is facing. 
        if (keyPrimaryDir != oldPrimaryDir || keyPrimaryDir != facing)
        {
            facing = keyPrimaryDir;
            // Restart the animation of the tile.
            animTileCurrTime = 0;
            animTileOffset   = 1;
        }

        // We'll let the "Update the position animation" code block handle setting up the position
        // animation.
        animCurrTime = 1;
        animMaxTime  = 1;
        animEndX     = 0;
        animEndY     = 0;
    }

    if (state == Still)
        return;

    // Update the tile animation.
        
    // Calculate how many milliseconds have passed since the start of the animation.
    animTileCurrTime += 1000 / fps;

    // Flip the offset between 0 and 1 every timeToMove/2 milliseconds.
    while (animTileCurrTime >= timeToMove/2)
    {
        animTileCurrTime -= timeToMove/2;
        animTileOffset    = 1 - animTileOffset;
    }

    // Update the position animation.
    
    // We will interpolate the position between 0 and animEnd over a period of animMaxTime.
    // After animMaxTime has elapsed we will set up a new interpolation to continue the animation.
    std::int32_t oldAnimX = animEndX * animCurrTime / animMaxTime;
    std::int32_t oldAnimY = animEndY * animCurrTime / animMaxTime;
    std::int32_t newAnimX = 0;
    std::int32_t newAnimY = 0;

    // Calculate how many milliseconds have passed since the start of the animation.
    animCurrTime += 1000 / fps;

    // Check if the animation has ended.
    while (animCurrTime >= animMaxTime)
    {
        animCurrTime -= animMaxTime;
        newAnimX = animEndX;
        newAnimY = animEndY;

        // Set up a new animation interpolation.
        if (keySecondaryDir == None)
            animMaxTime = timeToMove; 
        else
        {
            // Remember that moving diagonally takes longer.
            // 4/3 is approximately the square root of 2.
            // Better approximations feel slower and sluggish (e.g. 14/10).
            animMaxTime = timeToMove * 4/3;
        }
            
        animEndX = 0;
        animEndY = 0;

        for (Dir dir : {keyPrimaryDir, keySecondaryDir})
        {
            if (dir == Up)
                animEndY -= tileSizeInPixels;
            if (dir == Right)
                animEndX += tileSizeInPixels;
            if (dir == Down)
                animEndY += tileSizeInPixels;
            if (dir == Left)
                animEndX -= tileSizeInPixels;
        }
    }

    // Interpolate to get new anim coordinates.
    newAnimX += animEndX * animCurrTime / animMaxTime;
    newAnimY += animEndY * animCurrTime / animMaxTime;

    // Work out the deltas.
    std::int32_t deltaAnimX = newAnimX - oldAnimX;
    std::int32_t deltaAnimY = newAnimY - oldAnimY;

    // Update x position.
    x += deltaAnimX;

    // Force the character to not intersect with foreground tiles.
    for (std::int32_t yIndexPos=0; yIndexPos<area->heightInTiles; ++yIndexPos)
    for (std::int32_t xIndexPos=0; xIndexPos<area->widthInTiles;  ++xIndexPos)
    {
        if (area->GetForegroundTileIndex(xIndexPos, yIndexPos) == TileData::NotFound)
            continue; // No blocking tile.

        const auto& t = tileSizeInPixels;
        const auto  xTilePos = xIndexPos*t;
        const auto  yTilePos = yIndexPos*t;
        if (x+t<=xTilePos || xTilePos+t<=x || y+t<=yTilePos || yTilePos+t<=y)
            continue; // Not intersecting.

        // Require either x <= xTilePos-t or xTilePos+t <= x, check which is easier to achieve.
        if (abs(xTilePos-t-x) < abs(xTilePos+t-x))
            x = xTilePos - t;
        else
            x = xTilePos + t;
    }

    // Update y position.
    y += deltaAnimY;

    // Force the character to not intersect with foreground tiles.
    for (std::int32_t yIndexPos=0; yIndexPos<area->heightInTiles; ++yIndexPos)
    for (std::int32_t xIndexPos=0; xIndexPos<area->widthInTiles;  ++xIndexPos)
    {
        if (area->GetForegroundTileIndex(xIndexPos, yIndexPos) == TileData::NotFound)
            continue; // No blocking tile.

        const auto& t = tileSizeInPixels;
        const auto  xTilePos = xIndexPos*t;
        const auto  yTilePos = yIndexPos*t;
        if (x+t<=xTilePos || xTilePos+t<=x || y+t<=yTilePos || yTilePos+t<=y)
            continue; // Not intersecting.

        // Require either y <= yTilePos-t or yTilePos+t <= y, check which is easier to achieve.
        if (abs(yTilePos-t-y) < abs(yTilePos+t-y))
            y = yTilePos - t;
        else
            y = yTilePos + t;
    }

    // Force the character to stay on the screen.
    x = std::clamp(x, 0, area->widthInTiles *tileSizeInPixels-tileSizeInPixels);
    y = std::clamp(y, 0, area->heightInTiles*tileSizeInPixels-tileSizeInPixels);
    return;
}

void Hero::SetDirFromKeys(const KeyStates& keyStates)
{
    // Create a temporary list of key presses.
    std::array<bool, 4> keysPressed;
    keysPressed[Up]    = keyStates.upPressed;
    keysPressed[Right] = keyStates.rightPressed;
    keysPressed[Down]  = keyStates.downPressed;
    keysPressed[Left]  = keyStates.leftPressed;
       
    // Priority of keys.
    std::array<Dir, 6> priority = {keyPrimaryDir, keySecondaryDir, Up, Down, Right, Left};

    // Reset.
    keyPrimaryDir   = None;
    keySecondaryDir = None;

    // Choose the primary and secondary key presses according to the priority list.
    for (Dir dir : priority)
    {
        if (dir == None || !keysPressed[dir])
            continue;

        if (keyPrimaryDir != None)
        {
            keySecondaryDir = dir;
            return;
        }
       
        keyPrimaryDir = dir;

        // For the purposes of determining the secondary direction, ignore dir and the opposite
        // direction of dir.
        if (dir == Up || dir == Down)
        {
            keysPressed[Up]   = false;
            keysPressed[Down] = false;
        }
        else
        {
            keysPressed[Left]  = false;
            keysPressed[Right] = false;
        }
    }

    return;
}