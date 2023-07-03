#include "Hero.h"
#include "CollisionDetection.h"
#include <algorithm>
#include <array>
#include <vector>

Hero::Hero(const TileData& tileData, const Area& area, std::int32_t fps)
{
    this->area = &area;
    this->tileData = &tileData;
    frameIntervalMicrosec = 1000000 / fps;
    timeToMoveAPixelOrthogonallyMicrosec = timeToMoveATileOrthogonallyMicrosec
                                           / tileData.GetTileSizeInPixels();
    timeToMoveAPixelDiagonallyMicrosec = timeToMoveAPixelOrthogonallyMicrosec * 4/3;

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
    //return tileData->GetTileIndex("TestTiles/BaseHero");
    std::int32_t faceOffset = 2 * static_cast<std::int32_t>(facing);
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
            animTileOffset = 0;
            animTileTimeMicrosec = 0;
        }
        else
        {      
            state = Moving;

            // Check if we need to change the direction the character is facing. 
            if (keyPrimaryDir != oldPrimaryDir || keyPrimaryDir != facing)
            {
                facing = keyPrimaryDir;
                // Restart the animation of the tile.
                animTileOffset = 1;
                animTileTimeMicrosec = 0;
            }
        }
    }

    if (state == Still)
    {
        animMoveTimeOverflowMicrosec = 0;
        return;
    }

    // Update the tile animation.
        
    // Calculate how many milliseconds have passed since the start of the animation.
    animTileTimeMicrosec += frameIntervalMicrosec;

    // Flip the offset between 0 and 1 every timeToMoveATileOrthogonallyMicrosec/2 microseconds.
    while (animTileTimeMicrosec >= timeToMoveATileOrthogonallyMicrosec/2)
    {
        animTileTimeMicrosec -= timeToMoveATileOrthogonallyMicrosec/2;
        animTileOffset        = 1 - animTileOffset;
    }

    // Update the position animation.

    const TileBaseRect& heroBase = tileData->GetTileBaseRect(GetTileIndex());
    bool determinedGlanceDir = false;
    Dir  glanceDir = None;

    // Use up some time waiting to account for the movement that happened in the last frame.
    std::int32_t timeLeftInMicrosec = frameIntervalMicrosec - animMoveTimeOverflowMicrosec;
    while (timeLeftInMicrosec > 0)
    {
        if (TryMoving(keyPrimaryDir, keySecondaryDir, timeLeftInMicrosec, heroBase))
        {
            determinedGlanceDir = false;
            continue;
        }

        // Check if we are:
        // - moving diagonally but can remove a direction to avoid being blocked
        // - moving orthogonally but can glance off a block. 

        if (keySecondaryDir != None)
        {
            if (TryMoving(keyPrimaryDir, None, timeLeftInMicrosec, heroBase))
                continue;

            if (TryMoving(keySecondaryDir, None, timeLeftInMicrosec, heroBase))
                continue;
        }
        else
        {
            if (!determinedGlanceDir)
            {
                glanceDir = DetermineGlanceDir(keyPrimaryDir, heroBase);
                determinedGlanceDir = true;
            }

            if (glanceDir!=None && TryMoving(glanceDir, None, timeLeftInMicrosec, heroBase))
                continue;
        }

        // Fully blocked.
        timeLeftInMicrosec = 0;
        break;
    }

    // Do the rest of the waiting in the next frame.
    animMoveTimeOverflowMicrosec = -timeLeftInMicrosec;
    
    // Force the character to stay on the screen.
    x = std::clamp(x, 0, (area->widthInTiles-1)  * tileData->GetTileSizeInPixels());
    y = std::clamp(y, 0, (area->heightInTiles-1) * tileData->GetTileSizeInPixels());
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

void Hero::UpdateOffsetsFromDir(std::int32_t& offsetX, std::int32_t& offsetY, Dir dir)
{
    switch (dir)
    {
    case Up:
        offsetY -= 1;
        break;
    case Down:
        offsetY += 1;
        break;
    case Left:
        offsetX -= 1;
        break;
    case Right:
        offsetX += 1;
        break;
    default:
        break;
    }
}

bool Hero::TryMoving(Dir dir1, Dir dir2, std::int32_t& timeLeft, const TileBaseRect& base)
{
    std::int32_t offsetX = 0;
    std::int32_t offsetY = 0;

    UpdateOffsetsFromDir(offsetX, offsetY, dir1);
    UpdateOffsetsFromDir(offsetX, offsetY, dir2);
    if (IntersectingAgainstForeground(x+offsetX, y+offsetY, base, *area, *tileData))
        return false;

    // Update.
    x += offsetX;
    y += offsetY;

    if (dir1 == None || dir2 == None)
        timeLeft -= timeToMoveAPixelOrthogonallyMicrosec;
    else
        timeLeft -= timeToMoveAPixelDiagonallyMicrosec;

    return true;
}

Hero::Dir Hero::DetermineGlanceDir(Dir primaryDir, const TileBaseRect& base)
{
    // Try the two directions at right angles to the primaryDir.
    Dir glanceDir[2];
    glanceDir[0] = static_cast<Dir>((static_cast<std::int32_t>(primaryDir) + 1) % 4);
    glanceDir[1] = static_cast<Dir>((static_cast<std::int32_t>(primaryDir) + 3) % 4);

    // Distance of -1 represents a failure.
    std::int32_t glanceDistance[2] = {-1, -1};

    std::int32_t pX = 0;
    std::int32_t pY = 0;
    UpdateOffsetsFromDir(pX, pY, primaryDir);

    ObjectsIntersecting blocking = 
        ListOfIntersectingForegroundObjects(x+pX, y+pY, base, *area, *tileData);

    for (std::int32_t i=0; i<2; ++i)
    {
        // Attempt to glance in glanceDir[i].
        const Dir& dir = glanceDir[i];

        std::int32_t maxGlanceDist     = 0;
        std::int32_t allowedGlanceDist = 0;
        for (std::int32_t j=0; j<blocking.numberFound; ++j)
        {
            const DynamicArea::Object& obj = area->foreground.objects[blocking.objectIndices[j]];
            const TileBaseRect& blockingBase = tileData->GetTileBaseRect(obj.tile);
            
            std::int32_t glanceDist;
            switch (dir)
            {
            case Up:
                // Distance to match the bottom of the bases
                // + height of the blocking obj to get the character to move over it.
                glanceDist = (base.bottomY + y) - (blockingBase.bottomY + obj.y) 
                            + blockingBase.height;
                break;
            case Down:
                // Distance to match the bottom of the bases
                // + height of the character to move under the block.
                glanceDist = (blockingBase.bottomY + obj.y) - (base.bottomY + y) + base.height;
                break;
            case Left:
                // Distance to match the left of the bases
                // + width of the character to move left of the block.
                glanceDist = (base.leftX + x) - (blockingBase.leftX + obj.x) + base.width;
                break;
            case Right:
                // Distance to match the left of the bases
                // + width of the blocking obj to move right of the block.
                glanceDist = (blockingBase.leftX + obj.x) - (base.leftX + x) + blockingBase.width;
                break;
            }

            if (maxGlanceDist < glanceDist)
            {
                maxGlanceDist = glanceDist;
                allowedGlanceDist = tileData->GetTileMetadata(obj.tile).glanceDist;
            }
        }

        if (maxGlanceDist > allowedGlanceDist)
            continue; // Failed. Try a different glance direction.

        // Check we aren't blocked at the target position (after we glance and move a single pixel
        // in the primaryDir).
        std::int32_t gX = 0;
        std::int32_t gY = 0;
        
        UpdateOffsetsFromDir(gX, gY, dir);
        
        gX *= maxGlanceDist;
        gY *= maxGlanceDist;
                
        if (IntersectingAgainstForeground(x+pX+gX, y+pY+gY, base, *area, *tileData))
            continue; // Failed, would be blocked.

        // Check we aren't blocked whilst glancing to the target position.
        TileBaseRect baseIncreased = base;
        switch (dir)
        {
        case Up:
            baseIncreased.height += maxGlanceDist;
            break;
        case Down:
            baseIncreased.height  += maxGlanceDist;
            baseIncreased.bottomY += maxGlanceDist;
            break;
        case Left:
            baseIncreased.width += maxGlanceDist;
            baseIncreased.leftX -= maxGlanceDist;
            break;
        case Right:
            baseIncreased.width += maxGlanceDist;
            break;
        }

        if (IntersectingAgainstForeground(x, y, baseIncreased, *area, *tileData))
            continue; // Failed, would be blocked.
        
        // Succeeded.
        glanceDistance[i] = maxGlanceDist;
    }

    if (glanceDistance[0] == -1 && glanceDistance[1] == -1)
        return None; // No valid choices, can't glance.

    if (glanceDistance[0] == -1)
        return glanceDir[1]; // Only valid choice.

    if (glanceDistance[1] == -1)
        return glanceDir[0]; // Only valid choice.
        
    // Both choices are valid, choose the shorter distance.
    return (glanceDistance[0] > glanceDistance[1]) ? glanceDir[1] : glanceDir[0];
}