#include "Hero.h"
#include "CollisionDetection.h"
#include <algorithm>
#include <array>
#include <vector>

Hero::Hero(const SpeedData& speedData, const TileData& tileData, Area& area)
{
    speed = &speedData;

    this->area = &area;
    this->tileData = &tileData;

    tileIndicesStill[Up   ] = tileData.GetTileIndex("TestTiles/HeroFacingUpStanding");
    tileIndicesStill[Right] = tileData.GetTileIndex("TestTiles/HeroFacingRightStanding");
    tileIndicesStill[Down ] = tileData.GetTileIndex("TestTiles/HeroFacingDownStanding");
    tileIndicesStill[Left ] = tileData.GetTileIndex("TestTiles/HeroFacingLeftStanding");

    tileIndicesWalking[Up   ][0] = tileData.GetTileIndex("TestTiles/HeroFacingUpMoving");
    tileIndicesWalking[Up   ][1] = tileData.GetTileIndex("TestTiles/HeroFacingUpStanding");
    tileIndicesWalking[Right][0] = tileData.GetTileIndex("TestTiles/HeroFacingRightMoving");
    tileIndicesWalking[Right][1] = tileData.GetTileIndex("TestTiles/HeroFacingRightStanding");
    tileIndicesWalking[Down ][0] = tileData.GetTileIndex("TestTiles/HeroFacingDownMoving");
    tileIndicesWalking[Down ][1] = tileData.GetTileIndex("TestTiles/HeroFacingDownStanding");
    tileIndicesWalking[Left ][0] = tileData.GetTileIndex("TestTiles/HeroFacingLeftMoving");
    tileIndicesWalking[Left ][1] = tileData.GetTileIndex("TestTiles/HeroFacingLeftStanding");

    tileIndicesPushing[Up   ][0] = tileData.GetTileIndex("TestTiles/HeroFacingUpPushingA");
    tileIndicesPushing[Up   ][1] = tileData.GetTileIndex("TestTiles/HeroFacingUpPushingB");
    tileIndicesPushing[Right][0] = tileData.GetTileIndex("TestTiles/HeroFacingRightPushingA");
    tileIndicesPushing[Right][1] = tileData.GetTileIndex("TestTiles/HeroFacingRightPushingB");
    tileIndicesPushing[Down ][0] = tileData.GetTileIndex("TestTiles/HeroFacingDownPushingA");
    tileIndicesPushing[Down ][1] = tileData.GetTileIndex("TestTiles/HeroFacingDownPushingB");
    tileIndicesPushing[Left ][0] = tileData.GetTileIndex("TestTiles/HeroFacingLeftPushingA");
    tileIndicesPushing[Left ][1] = tileData.GetTileIndex("TestTiles/HeroFacingLeftPushingB");

    // Compute maxBufferDistance array based on how much space there is around the base.
    auto base = tileData.GetTileBaseRect(GetTileIndex());
    auto tileSize = tileData.GetTileSizeInPixels();

    pushingData.maxBufferDistance[Up]    = base.bottomY - base.height + 1;
    pushingData.maxBufferDistance[Down]  = tileSize - base.bottomY - 1;
    pushingData.maxBufferDistance[Left]  = tileSize - base.leftX - base.width;
    pushingData.maxBufferDistance[Right] = base.leftX;
    pushingData.maxBufferDistance[None]  = 0;

    // Allow a bit of overlap with the tile.
    for (auto& dist : pushingData.maxBufferDistance)
        dist = std::max(dist-2, 0);

    return;
}

std::int32_t Hero::GetTileIndex()
{
    //return tileData->GetTileIndex("TestTiles/BaseHero"); // For debugging.

    if (state == Walking)
        return tileIndicesWalking[facing][animTileFrame];
    
    if (state == Pushing)
        return tileIndicesPushing[facing][animTileFrame];

    return tileIndicesStill[facing];
}

void Hero::Update(const KeyStates& keyStates)
{
    State oldState = state;

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
            ResetAnimation();
            ClearAllPushingData();
        }
        else
        {
            if (oldState == Still)
                state = Walking;

            // Check if we need to change the direction the character is facing. 
            if (keyPrimaryDir != oldPrimaryDir || keyPrimaryDir != facing)
            {
                state  = Walking;
                facing = keyPrimaryDir;
                ResetAnimation();
                ClearAllPushingData();
            }
        }
    }

    if (state == Still)
    {
        animMoveTimeOverflowMicrosec = 0;
        return;
    }

    // Update the tile animation.
        
    // Calculate how many microseconds have passed since the start of the animation.
    animTileTimeMicrosec += speed->frameIntervalMicrosec;

    // Flip the offset between 0 and 1 every timeToMoveATileOrthogonallyMicrosec/2 microseconds.
    while (animTileTimeMicrosec >= speed->timeToMoveHeroATileOrthogonallyMicrosec/2)
    {
        animTileTimeMicrosec -= speed->timeToMoveHeroATileOrthogonallyMicrosec/2;
        animTileFrame = 1 - animTileFrame;
    }

    // Update the position animation.

    // Use up some time waiting to account for the movement that happened in the last frame.

    std::int32_t heroNextEventTimeMicrosec  = animMoveTimeOverflowMicrosec;
    std::int32_t blockNextEventTimeMicrosec = 0;

    if (pushingData.isPushingObject)
        blockNextEventTimeMicrosec = pushingData.block->animMoveTimeMicrosec;

    while (true)
    {
        // Update currentTime.
        std::int32_t currentTimeMicrosec = heroNextEventTimeMicrosec;
        if (pushingData.isPushingObject)
            currentTimeMicrosec = std::min(currentTimeMicrosec, blockNextEventTimeMicrosec);

        if (currentTimeMicrosec >= speed->frameIntervalMicrosec)
            break;

        // Update the block before updating the character.

        // Update pushing object.
        if (pushingData.isPushingObject && currentTimeMicrosec == blockNextEventTimeMicrosec)
            UpdatePushingObject(blockNextEventTimeMicrosec);

        // Update the character (and the pushing data).
        if (currentTimeMicrosec == heroNextEventTimeMicrosec)
            UpdateCharacter(heroNextEventTimeMicrosec, blockNextEventTimeMicrosec);
    }

    // Do rest of the waiting in the next frame.
    animMoveTimeOverflowMicrosec = heroNextEventTimeMicrosec - speed->frameIntervalMicrosec;
        
    if (pushingData.isPushingObject)
        pushingData.block->animMoveTimeMicrosec = blockNextEventTimeMicrosec;

    if (pushingData.listOfPushed.numberFound == 0 && !pushingData.isPushingObject)
        state = Walking;

    if (oldState != state)
        ResetAnimation();

    // TODO: Remove (should use walls in the layout to constrain movement).
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

bool Hero::TryMoving(Dir dir1, Dir dir2, std::int32_t& waitTimeMicrosec, const TileBaseRect& base)
{
    std::int32_t offsetX = 0;
    std::int32_t offsetY = 0;

    UpdateOffsetsFromDir(offsetX, offsetY, dir1);
    UpdateOffsetsFromDir(offsetX, offsetY, dir2);
    if (IsCollidingAgainstForeground(x+offsetX, y+offsetY, base, NoObject))
        return false;

    // Update.
    x += offsetX;
    y += offsetY;

    if (dir1 == None || dir2 == None)
        waitTimeMicrosec = speed->timeToMoveHeroAPixelOrthogonallyMicrosec;
    else
        waitTimeMicrosec = speed->timeToMoveHeroAPixelDiagonallyMicrosec;

    return true;
}

Hero::GlanceInfo Hero::DetermineGlanceInfo(Dir primaryDir,
                                           std::int32_t tileX,
                                           std::int32_t tileY,
                                           const TileBaseRect& base,
                                           std::int32_t excludeObject) const
{
    // Try the two directions at right angles to the primaryDir.
    auto glanceDirs = GetGlanceDirs(primaryDir);
    std::array<GlanceInfo, 2> glanceInfo = {};
    glanceInfo[0].glanceDir = glanceDirs[0];
    glanceInfo[1].glanceDir = glanceDirs[1];

    // Distance of -1 represents a failure.
    std::int32_t glanceDistance[2] = {-1, -1};

    std::int32_t pX = 0;
    std::int32_t pY = 0;
    UpdateOffsetsFromDir(pX, pY, primaryDir);

    ObjectsIntersecting blocking = FindIntersectingForegroundObjects(tileX+pX, tileY+pY, base,
                                                                     excludeObject);

    for (std::int32_t i=0; i<2; ++i)
    {
        // Attempt to glance in glanceInfo[i].glanceDir.
        const Dir& dir = glanceInfo[i].glanceDir;

        std::int32_t maxGlanceDist     = 0;
        std::int32_t allowedGlanceDist = 0;
        for (std::int32_t j=0; j<blocking.numberFound; ++j)
        {
            const Object& obj = area->foreground.objects[blocking.objectIndices[j]];
            const TileBaseRect& blockingBase = tileData->GetTileBaseRect(obj.tile);
            
            std::int32_t glanceDist = GetGlanceDist(dir, tileX, tileY, base,
                                                    obj.x, obj.y, blockingBase);
            if (maxGlanceDist < glanceDist)
            {
                maxGlanceDist = glanceDist;
                allowedGlanceDist = obj.blockProperties.glanceDist;
                glanceInfo[i].glancingObjectId = blocking.objectIndices[j];
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
                
        if (IsCollidingAgainstForeground(tileX+pX+gX, tileY+pY+gY, base, excludeObject))
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

        if (IsCollidingAgainstForeground(tileX, tileY, baseIncreased, excludeObject))
            continue; // Failed, would be blocked.
        
        // Succeeded.
        glanceDistance[i] = maxGlanceDist;
    }

    if (glanceDistance[0] == -1 && glanceDistance[1] == -1)
        return GlanceInfo{None, NoObject}; // No valid choices, can't glance.

    if (glanceDistance[0] == -1)
        return glanceInfo[1]; // Only valid choice.

    if (glanceDistance[1] == -1)
        return glanceInfo[0]; // Only valid choice.
        
    // Both choices are valid, choose the shorter distance.
    return (glanceDistance[0] > glanceDistance[1]) ? glanceInfo[1] : glanceInfo[0];
}

bool Hero::WithinPushRange(Dir primaryDir, const TileBaseRect& base,
                           std::int32_t glanceDist,
                           std::int32_t blockingX, std::int32_t blockingY,
                           const TileBaseRect& blockingBase) const
{
    // Try the two directions at right angles to the primaryDir.
    for (auto dir : GetGlanceDirs(primaryDir))
    {
        std::int32_t dist = GetGlanceDist(dir, x, y, base, blockingX, blockingY, blockingBase);
        if (dist <= glanceDist)
            return false;
    }

    return true;
}

std::array<Hero::Dir, 2> Hero::GetGlanceDirs(Dir dir)
{
    switch(dir)
    {
    case Up:    return {Right, Left };
    case Right: return {Down,  Up   };
    case Down:  return {Left,  Right};
    case Left:  return {Up,    Down };
    default:    return {None,  None };
    }
}

std::int32_t Hero::GetGlanceDist(Dir dir,
                                 std::int32_t x, std::int32_t y,
                                 const TileBaseRect& base,
                                 std::int32_t blockingX, std::int32_t blockingY,
                                 const TileBaseRect& blockingBase)
{
    switch (dir)
    {
    case Up:
        // Distance to match the bottom of the bases
        // + height of the blocking obj to get the character to move over it.
        return (base.bottomY + y) - (blockingBase.bottomY + blockingY) + blockingBase.height;
    case Down:
        // Distance to match the bottom of the bases
        // + height of the character to move under the block.
        return (blockingBase.bottomY + blockingY) - (base.bottomY + y) + base.height;
    case Left:
        // Distance to match the left of the bases
        // + width of the character to move left of the block.
        return (base.leftX + x) - (blockingBase.leftX + blockingX) + base.width;
    case Right:
        // Distance to match the left of the bases
        // + width of the blocking obj to move right of the block.
        return (blockingBase.leftX + blockingX) - (base.leftX + x) + blockingBase.width;
    default:
        return 0;
    }
}

void Hero::RemoveObjectFromList(ObjectsIntersecting& list, std::int32_t id) const
{
    std::int32_t readIndex = 0;
    std::int32_t writeIndex = 0;
    while (readIndex < list.numberFound)
    {
        if (list.objectIndices[readIndex] == id)
        {
            ++readIndex;
            continue;
        }

        if (readIndex != writeIndex)
            list.objectIndices[writeIndex] = list.objectIndices[readIndex];

        ++readIndex;
        ++writeIndex;
    }

    list.numberFound = writeIndex;
    return;
}

ObjectsIntersecting Hero::FindIntersectingForegroundObjects(std::int32_t tileX,
                                                            std::int32_t tileY,
                                                            const TileBaseRect& base,
                                                            std::int32_t excludeObject) const
{
    ObjectsIntersecting list = ListOfIntersectingForegroundObjects(tileX, tileY, base,
                                                                   excludeObject, *area, *tileData);

    if (!pushingData.isPushingObject
        || list.numberFound == list.maxObjects
        || pushingData.pushingObjectId == excludeObject)
    {
        return list;
    }

    // Check if it's already on the list.
    for (std::int32_t i=0; i<list.numberFound; ++i)
    {
        if (list.objectIndices[i] == pushingData.pushingObjectId)
            return list;
    }

    // Check to see if the pushing object would intersect.
    if (!IntersectsWithPushedObject(tileX, tileY, base))
        return list;

    // Add to the list.
    list.objectIndices[list.numberFound] = pushingData.pushingObjectId;
    ++list.numberFound;

    return list;
}

bool Hero::IsCollidingAgainstForeground(std::int32_t tileX,
                                        std::int32_t tileY,
                                        const TileBaseRect& base,
                                        std::int32_t excludeObject) const
{
    bool isIntersecting = IntersectingAgainstForeground(tileX, tileY, base,
                                                        excludeObject, *area, *tileData);

    if (isIntersecting
        || !pushingData.isPushingObject
        || pushingData.pushingObjectId == excludeObject)
    {
        return isIntersecting;
    }

    // Check to see if the pushing object would intersect.
    return IntersectsWithPushedObject(tileX, tileY, base);
}

bool Hero::IntersectsWithPushedObject(std::int32_t tileX,
                                      std::int32_t tileY,
                                      const TileBaseRect& base) const
{
    if (!pushingData.isPushingObject)
        return false;

    auto newBase = tileData->GetTileBaseRect(pushingData.pushingObject->tile);

    switch (pushingData.pushDir)
    {
    case Up:
        newBase.height  += pushingData.bufferDistance;
        newBase.bottomY += pushingData.bufferDistance;
        break;
    case Down:
        newBase.height  += pushingData.bufferDistance;
        break;
    case Left:
        newBase.width   += pushingData.bufferDistance;
        break;
    case Right:
        newBase.width   += pushingData.bufferDistance;
        newBase.leftX   -= pushingData.bufferDistance;
        break;
    default:
        break;
    }

    return BasesIntersect(tileX, tileY, base,
                          pushingData.pushingObject->x, pushingData.pushingObject->y, newBase);
}

ObjectsIntersecting Hero::GetListOfBlockers(const TileBaseRect& base) const
{
    std::int32_t newX = x;
    std::int32_t newY = y;
    UpdateOffsetsFromDir(newX, newY, keyPrimaryDir);

    ObjectsIntersecting list = FindIntersectingForegroundObjects(newX, newY, base, NoObject);
    return list;
}

ObjectsIntersecting Hero::GetListStillBlocking(const TileBaseRect& base,
                                               const ObjectsIntersecting& oldList) const
{
    std::int32_t newX = x;
    std::int32_t newY = y;
    UpdateOffsetsFromDir(newX, newY, keyPrimaryDir);

    ObjectsIntersecting newList;

    for (std::int32_t i=0; i<oldList.numberFound; ++i)
    {
        if (oldList.objectIndices[i] != pushingData.pushingObjectId)
        {
            const auto& obj = area->foreground.objects[oldList.objectIndices[i]];
            const auto& blockingBase = tileData->GetTileBaseRect(obj.tile);
            if (!BasesIntersect(newX, newY, base, obj.x, obj.y, blockingBase))
                continue;
        }
        else
        {
            if (!IntersectsWithPushedObject(newX, newY, base))
                continue;
        }

        std::int32_t index = newList.numberFound;
        newList.objectIndices[index] = oldList.objectIndices[i];
        ++newList.numberFound;
    }
    
    return newList;
}

void Hero::UpdatePushingInfo(ObjectsIntersecting&& listStillBlocking,
                             std::int32_t updateTimeInMicrosec)
{
    // Update the times.
    std::array<std::int32_t, ObjectsIntersecting::maxObjects> newPeriodPushed = {};

    for (std::int32_t i=0; i<listStillBlocking.numberFound; ++i)
    {
        newPeriodPushed[i] = updateTimeInMicrosec;
        for (std::int32_t j=0; j<pushingData.listOfPushed.numberFound; ++j)
        {
            if (pushingData.listOfPushed.objectIndices[j] == listStillBlocking.objectIndices[i])
                newPeriodPushed[i] += pushingData.periodPushedMicrosec[j];
        }
    }

    // Replace the lists.
    pushingData.listOfPushed         = std::move(listStillBlocking);
    pushingData.periodPushedMicrosec = std::move(newPeriodPushed);

    // Avoid overflow.
    for (std::int32_t i=0; i<pushingData.listOfPushed.numberFound; ++i)
    {           
        const auto& obj = area->foreground.objects[pushingData.listOfPushed.objectIndices[i]];
        pushingData.periodPushedMicrosec[i] = std::min(pushingData.periodPushedMicrosec[i],
                                                       obj.blockProperties.pushPeriodMicrosec);
    }

    return;
}

void Hero::ClearAllPushingData()
{
    pushingData.listOfPushed.numberFound = 0;
    ClearPushingObjectData();
    return;
}

void Hero::ClearPushingObjectData()
{
    if (!pushingData.isPushingObject)
        return;

    if (pushingData.block->type == BlockType::Unaligned)
        pushingData.block->isMoving = false;

    pushingData.isPushingObject = false;
    pushingData.pushingObjectId = NoObject;
    pushingData.pushingObject   = nullptr;
    pushingData.block           = nullptr;
    
    pushingData.bufferDistance = 0;
    pushingData.pushDir        = None;
    return;
}

void Hero::ResetAnimation()
{
    animTileFrame = 0;
    animTileTimeMicrosec = 0;
    return;
}

void Hero::UpdatePushingObject(std::int32_t& blockNextEventTimeMicrosec)
{
    if (   pushingData.block->type == BlockType::SinglePushGridAligned
        || pushingData.block->type == BlockType::MultiplePushGridAligned)
    {
        UpdatePushingObjectAligned(blockNextEventTimeMicrosec);
        return;
    }
    
    if (pushingData.block->type == BlockType::Unaligned) 
    {
        UpdatePushingObjectUnaligned(blockNextEventTimeMicrosec);
        return;
    }

    blockNextEventTimeMicrosec = speed->frameIntervalMicrosec;
    return;
}

void Hero::UpdatePushingObjectAligned(std::int32_t& blockNextEventTimeMicrosec)
{
    // We need to consider 3 scenarios:
    // (1) Block is moving and hasn't yet reached its target position.
    // (2) Block is moving and has just reached its target position.
    // (3) Block hasn't started moving yet.

    // Check if we are in (2) and either finish the update (if it is a one time push block) or make
    // it look like (3) by transitioning it to not moving (the handling for (2) and (3) will be
    // identical in the rest of the function).
    if (pushingData.block->isMoving
        && pushingData.pushingObject->x == pushingData.block->targetX
        && pushingData.pushingObject->y == pushingData.block->targetY)
    {
        pushingData.block->isMoving = false;
            
        if (pushingData.block->type == BlockType::SinglePushGridAligned)
        {
            pushingData.block->finishedMoving = true;

            if (pushingData.bufferDistance != 0)
            {
                DecrementPushBufferDistance();
                blockNextEventTimeMicrosec += 
                    pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
                return;
            }

            // Do nothing till next frame.
            blockNextEventTimeMicrosec = speed->frameIntervalMicrosec;
            return;
        }
    }

    // At this point we have:
    // (1)     Block is moving (and it hasn't yet reached its target position).
    // (2 & 3) Block is not moving (because either it just reached its target position, or it never
    //         started moving).

    // Decide if the block should transition from not moving to moving.
    if (!pushingData.block->isMoving)
    {
        pushingData.block->offsetX = 0;
        pushingData.block->offsetY = 0;
        UpdateOffsetsFromDir(pushingData.block->offsetX, pushingData.block->offsetY, keyPrimaryDir);

        pushingData.block->targetX = pushingData.pushingObject->x
            + pushingData.block->offsetX * tileData->GetTileSizeInPixels();
        pushingData.block->targetY = pushingData.pushingObject->y
            + pushingData.block->offsetY * tileData->GetTileSizeInPixels();

        pushingData.block->isMoving = true;

        // TODO: Remove (should use walls in the layout to constrain movement).
        // Check if trying to move block outside the area.
        const std::int32_t maxXPos = (area->widthInTiles-1)  * tileData->GetTileSizeInPixels();
        const std::int32_t maxYPos = (area->heightInTiles-1) * tileData->GetTileSizeInPixels();
        if (   pushingData.block->targetX != std::clamp(pushingData.block->targetX, 0, maxXPos)
            || pushingData.block->targetY != std::clamp(pushingData.block->targetY, 0, maxYPos))
        {
            pushingData.block->isMoving = false;

            if (pushingData.bufferDistance != 0)
            {
                DecrementPushBufferDistance();
                blockNextEventTimeMicrosec +=
                    pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
                return;
            }

            // Do nothing till next frame.
            blockNextEventTimeMicrosec = speed->frameIntervalMicrosec;
            return;
        }

        // Check if finished moving or intersecting with foreground.
        if (pushingData.block->finishedMoving
            || IsCollidingAgainstForeground(
                   pushingData.block->targetX,
                   pushingData.block->targetY,
                   tileData->GetTileBaseRect(pushingData.pushingObject->tile),
                   pushingData.pushingObjectId))
        {
            pushingData.block->isMoving = false;

            if (pushingData.bufferDistance != 0)
            {
                DecrementPushBufferDistance();
                blockNextEventTimeMicrosec +=
                    pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
                return;
            }

            // Do nothing till next frame.
            blockNextEventTimeMicrosec = speed->frameIntervalMicrosec;
            return;
        }
    }

    // Block is moving, update appropriately.
    IncrementPushBufferDistance();
    pushingData.pushingObject->x += pushingData.block->offsetX;
    pushingData.pushingObject->y += pushingData.block->offsetY;
    blockNextEventTimeMicrosec += pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
    return;
}

void Hero::UpdatePushingObjectUnaligned(std::int32_t& blockNextEventTimeMicrosec)
{
    pushingData.block->offsetX = 0;
    pushingData.block->offsetY = 0;
    UpdateOffsetsFromDir(pushingData.block->offsetX, pushingData.block->offsetY, keyPrimaryDir);

    pushingData.block->targetX = pushingData.pushingObject->x + pushingData.block->offsetX;
    pushingData.block->targetY = pushingData.pushingObject->y + pushingData.block->offsetY;

    pushingData.block->isMoving = true;

    // TODO: Remove (should use walls in the layout to constrain movement).
    // Check if trying to move block outside the area.
    const std::int32_t maxXPos = (area->widthInTiles-1)  * tileData->GetTileSizeInPixels();
    const std::int32_t maxYPos = (area->heightInTiles-1) * tileData->GetTileSizeInPixels();
    if (   pushingData.block->targetX != std::clamp(pushingData.block->targetX, 0, maxXPos)
        || pushingData.block->targetY != std::clamp(pushingData.block->targetY, 0, maxYPos))
    {
        pushingData.block->isMoving = false;

        if (pushingData.bufferDistance != 0)
        {
            DecrementPushBufferDistance();
            blockNextEventTimeMicrosec += pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
            return;
        }

        // Do nothing till next frame.
        blockNextEventTimeMicrosec = speed->frameIntervalMicrosec;
        return;
    }

    // Check if intersecting with foreground.
    if (IsCollidingAgainstForeground(pushingData.block->targetX,
                                     pushingData.block->targetY,
                                     tileData->GetTileBaseRect(pushingData.pushingObject->tile),
                                     pushingData.pushingObjectId))
    {

        // Check if can glance.
        GlanceInfo glanceInfo = DetermineGlanceInfo(
                                    keyPrimaryDir,
                                    pushingData.pushingObject->x,
                                    pushingData.pushingObject->y,
                                    tileData->GetTileBaseRect(pushingData.pushingObject->tile),
                                    pushingData.pushingObjectId);

        if (glanceInfo.glanceDir != None)
        {
            pushingData.block->offsetX = 0;
            pushingData.block->offsetY = 0;
            UpdateOffsetsFromDir(pushingData.block->offsetX,
                                 pushingData.block->offsetY,
                                 glanceInfo.glanceDir);

            pushingData.block->targetX = pushingData.pushingObject->x + pushingData.block->offsetX;
            pushingData.block->targetY = pushingData.pushingObject->y + pushingData.block->offsetY;

            pushingData.pushingObject->x = pushingData.block->targetX;
            pushingData.pushingObject->y = pushingData.block->targetY;
            
            blockNextEventTimeMicrosec += pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
            return;
        }
       
        // Blocked.
        pushingData.block->isMoving = false;
            
        if (pushingData.bufferDistance != 0)
        {
            DecrementPushBufferDistance();
            blockNextEventTimeMicrosec += pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
            return;
        }

        // Do nothing till next frame.
        blockNextEventTimeMicrosec = speed->frameIntervalMicrosec;
        return;
    }
                
    // Block is moving, update appropriately.
    IncrementPushBufferDistance();
    pushingData.pushingObject->x = pushingData.block->targetX;
    pushingData.pushingObject->y = pushingData.block->targetY;
    blockNextEventTimeMicrosec += pushingData.block->timeToMoveAPixelOrthogonallyMicrosec;
    return;
}

void Hero::UpdateCharacter(std::int32_t& heroNextEventTimeMicrosec,
                           std::int32_t& blockNextEventTimeMicrosec)
{
    ObjectsIntersecting listOfBlockers = StartTrackingListOfPushedObjects();

    MovementType movementType = FullyBlocked;
    GlanceInfo glanceInfo;
    const TileBaseRect& heroBase = tileData->GetTileBaseRect(GetTileIndex());

    // If we can't move we should use up the rest of the time
    // (set this as the default scenario).
    std::int32_t waitTimeMicrosec = speed->frameIntervalMicrosec - heroNextEventTimeMicrosec;

    if (pushingData.isPushingObject)
    {
        waitTimeMicrosec = std::min(waitTimeMicrosec,
                                    blockNextEventTimeMicrosec - heroNextEventTimeMicrosec);
    }

    // Check if we are:
    // - moving without being blocked
    // - moving diagonally but can remove a direction to avoid being blocked
    // - moving orthogonally but can glance off a block. 

    if (TryMoving(keyPrimaryDir, keySecondaryDir, waitTimeMicrosec, heroBase))
    {
        movementType = NotBlocked;
    }
    else
    if (keySecondaryDir != None)
    {
        if (TryMoving(keyPrimaryDir, None, waitTimeMicrosec, heroBase))
            movementType = ForcedPrimaryDir;
        else
        if (TryMoving(keySecondaryDir, None, waitTimeMicrosec, heroBase))
        {
            glanceInfo = DetermineGlanceInfo(keyPrimaryDir, x, y, heroBase, NoObject);
            if (glanceInfo.glanceDir == keySecondaryDir)
                movementType = Glancing;
            else
                movementType = ForcedSecondaryDir;
        }
        else
            movementType = FullyBlocked;
    }
    else
    {
        glanceInfo = DetermineGlanceInfo(keyPrimaryDir, x, y, heroBase, NoObject);

        if (glanceInfo.glanceDir != None
            && TryMoving(glanceInfo.glanceDir, None, waitTimeMicrosec, heroBase))
            movementType = Glancing;
        else
            movementType = FullyBlocked;
    }

    // Check if we've broken contact with pushed object because the character is blocked or moving
    // in the wrong direction.
    PossiblyBreakContactWithPushedObject(movementType, glanceInfo, heroBase,
                                         blockNextEventTimeMicrosec);

    // Update the waitTime according to the pushed object.
    ChangeWaitTimeToSyncWithPushedObject(movementType,
                                         heroBase,
                                         heroNextEventTimeMicrosec,
                                         blockNextEventTimeMicrosec,
                                         waitTimeMicrosec);

    heroNextEventTimeMicrosec += waitTimeMicrosec;

    UpdateListOfPushedObjects(listOfBlockers,
                              movementType,
                              heroNextEventTimeMicrosec,
                              blockNextEventTimeMicrosec,
                              waitTimeMicrosec);
    return;
}

ObjectsIntersecting Hero::StartTrackingListOfPushedObjects()
{
    const TileBaseRect& heroBase = tileData->GetTileBaseRect(GetTileIndex());
    return GetListOfBlockers(heroBase);
}

void Hero::UpdateListOfPushedObjects(const ObjectsIntersecting& listOfBlockers,
                                     MovementType  movementType,
                                     std::int32_t  heroNextEventTimeMicrosec,
                                     std::int32_t& blockNextEventTimeMicrosec,
                                     std::int32_t  waitTimeMicrosec)
{
    const TileBaseRect& heroBase = tileData->GetTileBaseRect(GetTileIndex());
    ObjectsIntersecting listOfStillBlocking = GetListStillBlocking(heroBase, listOfBlockers);
    UpdatePushingInfo(std::move(listOfStillBlocking), waitTimeMicrosec);

    if (movementType == Glancing)
        pushingData.listOfPushed.numberFound = 0;

    bool uniquePushingObjectFound = true;
    std::int32_t objectId = NoObject;

    for (std::int32_t i=0; i<pushingData.listOfPushed.numberFound; ++i)
    {
        const auto& obj = area->foreground.objects[pushingData.listOfPushed.objectIndices[i]];
        const auto& block = obj.blockProperties;
        const auto& metadata = tileData->GetTileMetadata(obj.tile);
        const std::int32_t threshold = block.pushPeriodMicrosec;
        if (pushingData.periodPushedMicrosec[i] >= threshold)
        {
            state = Pushing;

            if (WithinPushRange(keyPrimaryDir, heroBase, block.glanceDist,
                    obj.x, obj.y, tileData->GetTileBaseRect(obj.tile)))
            {
                if (objectId == NoObject)
                    objectId = pushingData.listOfPushed.objectIndices[i];
                else
                    uniquePushingObjectFound = false;
            }
        }
    }

    if (uniquePushingObjectFound
        && objectId != NoObject
        && !area->foreground.objects[objectId].blockProperties.isMoving
        && !pushingData.isPushingObject)
    {
        pushingData.isPushingObject = true;
        pushingData.pushingObjectId = objectId;
        pushingData.pushingObject   = &area->foreground.objects[pushingData.pushingObjectId];
        pushingData.block           = &pushingData.pushingObject->blockProperties;

        pushingData.bufferDistance = 0;
        pushingData.pushDir        = keyPrimaryDir;

        blockNextEventTimeMicrosec = std::max(pushingData.block->animMoveTimeMicrosec,
                                              heroNextEventTimeMicrosec);
    }

    return;
}

void Hero::PossiblyBreakContactWithPushedObject(MovementType movementType,
                                                const GlanceInfo& glanceInfo,
                                                const TileBaseRect& base,
                                                std::int32_t blockNextEventTimeMicrosec)
{
    if (!pushingData.isPushingObject)
        return;

    bool breakContact = false;

    if (!WithinPushRange(keyPrimaryDir,
                         base,
                         pushingData.block->glanceDist,
                         pushingData.pushingObject->x,
                         pushingData.pushingObject->y,
                         tileData->GetTileBaseRect(pushingData.pushingObject->tile)))
    {
        breakContact = true;
    }

    if (movementType == FullyBlocked || movementType == ForcedSecondaryDir)
    {
        // Check if we are being impeded by something other than the object being pushed.
        ObjectsIntersecting intersectingList = GetListOfBlockers(base);
        bool foundNonPushingObject = false;
        bool foundPushingObject    = false;
        for (int i=0; i<intersectingList.numberFound; ++i)
        {
            if (intersectingList.objectIndices[i] != pushingData.pushingObjectId)
                foundNonPushingObject = true;
            else
                foundPushingObject = true;
        }

        if (foundNonPushingObject && !foundPushingObject)
            breakContact = true;
    }
        
    if (movementType == Glancing)
    {
        // Check glance dir is towards the pushed block by checking the midpoints of the bases.
        // (We scale the midpoints by a factor of 2 to avoid division.)
        const auto&  glanceObj  = area->foreground.objects[glanceInfo.glancingObjectId];
        TileBaseRect glanceBase = tileData->GetTileBaseRect(glanceObj.tile);
        TileBaseRect blockBase  = tileData->GetTileBaseRect(pushingData.pushingObject->tile);
        std::int32_t blockObjX  = pushingData.pushingObject->x;
        std::int32_t blockObjY  = pushingData.pushingObject->y;

        std::int32_t midBlockX  = 2*(blockObjX   + blockBase.leftX   ) + blockBase.width;
        std::int32_t midBlockY  = 2*(blockObjY   + blockBase.bottomY ) - blockBase.height;
        std::int32_t midGlanceX = 2*(glanceObj.x + glanceBase.leftX  ) + glanceBase.width;
        std::int32_t midGlanceY = 2*(glanceObj.y + glanceBase.bottomY) - glanceBase.height;

        if (   (glanceInfo.glanceDir == Left  && midBlockX >= midGlanceX)
            || (glanceInfo.glanceDir == Right && midBlockX <= midGlanceX)
            || (glanceInfo.glanceDir == Up    && midBlockY >= midGlanceY)
            || (glanceInfo.glanceDir == Down  && midBlockY <= midGlanceY))
        {
            breakContact = true;
        }
    }

    if (breakContact)
    {
        pushingData.block->animMoveTimeMicrosec = blockNextEventTimeMicrosec;
        ClearPushingObjectData(); 
    }

    return;
}

void Hero::ChangeWaitTimeToSyncWithPushedObject(MovementType movementType,
                                                const TileBaseRect& base,
                                                std::int32_t  heroNextEventTimeMicrosec,
                                                std::int32_t  blockNextEventTimeMicrosec,
                                                std::int32_t& waitTimeMicrosec)
{
    if (!pushingData.isPushingObject)
        return;
 
    if (!pushingData.block->isMoving && pushingData.bufferDistance == 0)
        return;

    // Double the movement speed of the character (by halving the waitTime and rounding up
    // to avoid 0). Either the character needs to catch up to the pushing object or it will
    // sync with it (in which case the speed up won't really matter).
    if (movementType != FullyBlocked)
        waitTimeMicrosec = (waitTimeMicrosec + 1)/2;

    // Check if the pushing object is in the way of the character moving in the primary
    // direction.
    std::int32_t newX = x;
    std::int32_t newY = y;
    UpdateOffsetsFromDir(newX, newY, keyPrimaryDir);

    if (IntersectsWithPushedObject(newX, newY, base))
    {
        waitTimeMicrosec = std::max(waitTimeMicrosec,
                                    blockNextEventTimeMicrosec - heroNextEventTimeMicrosec);
    }

    return;
}

void Hero::IncrementPushBufferDistance()
{   
    pushingData.bufferDistance = std::min(pushingData.bufferDistance + 1,
                                          pushingData.maxBufferDistance[pushingData.pushDir]);
    return;
}

void Hero::DecrementPushBufferDistance()
{
    pushingData.bufferDistance = std::max(pushingData.bufferDistance - 1, 0);
    return;
}
