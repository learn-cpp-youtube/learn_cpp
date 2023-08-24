#pragma once

#include "AreaData.h"
#include "TileData.h"
#include "SpeedData.h"
#include "KeyStates.h"
#include "CollisionDetection.h"
#include <array>
#include <cstdint>

class Hero
{
public:
    Hero() = default;

    Hero(const SpeedData& speedData, const TileData& tileData, Area& area);

    std::int32_t GetTileIndex();
    std::int32_t GetXPos() { return x; }
    std::int32_t GetYPos() { return y; }

    void Update(const KeyStates& keyStates);

private:
    static const std::int32_t NoObject = -1;

    // The enum values are used to index into an array, so it is important the direction values are
    // 0-3.
    enum Dir
    {
        Up    = 0,
        Right = 1,
        Down  = 2,
        Left  = 3,
        None  = 4
    };

    enum State
    {
        Still,
        Walking,
        Pushing
    };
    
    struct GlanceInfo
    {
        Dir glanceDir = None;
        std::int32_t glancingObjectId = NoObject;
    };

    enum MovementType
    {
        FullyBlocked,
        NotBlocked,
        ForcedPrimaryDir,
        ForcedSecondaryDir,
        Glancing
    };

    void SetDirFromKeys(const KeyStates& keyStates);
    static void UpdateOffsetsFromDir(std::int32_t& offsetX, std::int32_t& offsetY, Dir dir);
    
    // Returns true if suceeded (and updates x, y).
    // Returns false if failed (no updates are done).
    bool TryMoving(Dir dir1, Dir dir2, std::int32_t& waitTimeMicrosec, const TileBaseRect& base);

    // Returns None for the glanceDir if can't glance.
    GlanceInfo DetermineGlanceInfo(Dir primaryDir,
                                   std::int32_t tileX,
                                   std::int32_t tileY,
                                   const TileBaseRect& base,
                                   std::int32_t excludeObject) const;

    bool WithinPushRange(Dir primaryDir, const TileBaseRect& base,
                         std::int32_t glanceDist,
                         std::int32_t blockingX, std::int32_t blockingY,
                         const TileBaseRect& blockingBase) const;

    static std::array<Dir, 2> GetGlanceDirs(Dir dir);
    static std::int32_t GetGlanceDist(Dir dir,
                                      std::int32_t x, std::int32_t y,
                                      const TileBaseRect& base,
                                      std::int32_t blockingX, std::int32_t blockingY,
                                      const TileBaseRect& blockingBase);

    void RemoveObjectFromList(ObjectsIntersecting& list, std::int32_t id) const;

    // Accounts for the push buffer distance.
    ObjectsIntersecting FindIntersectingForegroundObjects(std::int32_t tileX,
                                                          std::int32_t tileY,
                                                          const TileBaseRect& base,
                                                          std::int32_t excludeObject) const;

    // Accounts for the push buffer distance.
    bool IsCollidingAgainstForeground(std::int32_t tileX,
                                      std::int32_t tileY,
                                      const TileBaseRect& base,
                                      std::int32_t excludeObject) const;

    // Accounts for push buffer distance.
    bool IntersectsWithPushedObject(std::int32_t tileX,
                                    std::int32_t tileY,
                                    const TileBaseRect& base) const;

    ObjectsIntersecting GetListOfBlockers(const TileBaseRect& base) const;
    ObjectsIntersecting GetListStillBlocking(const TileBaseRect& base,
                                             const ObjectsIntersecting& oldList) const;
    void UpdatePushingInfo(ObjectsIntersecting&& listStillBlocking,
                           std::int32_t updateTimeInMicrosec);

    void ClearAllPushingData();
    void ClearPushingObjectData();
    void ResetAnimation();

    void UpdatePushingObject(std::int32_t& blockNextEventTimeMicrosec);
    void UpdatePushingObjectAligned(std::int32_t& blockNextEventTimeMicrosec);
    void UpdatePushingObjectUnaligned(std::int32_t& blockNextEventTimeMicrosec);

    void UpdateCharacter(std::int32_t& heroNextEventTimeMicrosec,
                         std::int32_t& blockNextEventTimeMicrosec);
    ObjectsIntersecting StartTrackingListOfPushedObjects();
    void UpdateListOfPushedObjects(const ObjectsIntersecting& listOfBlockers,
                                   MovementType movementType,
                                   std::int32_t  heroNextEventTimeMicrosec,
                                   std::int32_t& blockNextEventTimeMicrosec,
                                   std::int32_t  waitTimeMicrosec);
    void PossiblyBreakContactWithPushedObject(MovementType movementType,
                                              const GlanceInfo& glanceInfo,
                                              const TileBaseRect& base,
                                              std::int32_t blockNextEventTimeMicrosec);
    void ChangeWaitTimeToSyncWithPushedObject(MovementType movementType,
                                              const TileBaseRect& base,
                                              std::int32_t  heroNextEventTimeMicrosec,
                                              std::int32_t  blockNextEventTimeMicrosec,
                                              std::int32_t& waitTimeMicrosec);

    void IncrementPushBufferDistance();
    void DecrementPushBufferDistance();

    State        state  = Still;
    Dir          facing = Down;
    std::int32_t x      = 0;
    std::int32_t y      = 0;

    // Keeps track of the last key presses.
    Dir keyPrimaryDir   = None;
    Dir keySecondaryDir = None;

    // Animation timer to ensure smooth movements between frames.
    std::int32_t animMoveTimeOverflowMicrosec = 0;

    // Animation timers to correctly animate the tile.
    std::int32_t animTileTimeMicrosec = 0;
    std::int32_t animTileFrame        = 0; // Flips between 0 and 1

    struct PushingData
    {
        // Data used to transition into the pushing state, and to determine the pushing object.
        ObjectsIntersecting listOfPushed; // Not necessarily moving.
        std::array<std::int32_t, ObjectsIntersecting::maxObjects> periodPushedMicrosec = {};

        // Data on the pushed object.
        bool             isPushingObject = false;
        std::int32_t     pushingObjectId = NoObject;
        Object*          pushingObject   = nullptr;
        BlockProperties* block           = nullptr;

        // Data used to force the character tile to be separated from the pushed object to make the
        // animation look better. The space between the character and the pushed object we term the
        // buffer.
        std::int32_t bufferDistance = 0;
        Dir          pushDir        = None;
        std::array<std::int32_t, 5> maxBufferDistance = {}; // Indexed by pushDir.
    };

    PushingData pushingData;

    // The tiles are in the order [Up, Right, Down, Left]
    // (note that this matches the order of the Dir enum).
    std::array<std::int32_t, 4>                tileIndicesStill   = {}; // [Dir]
    std::array<std::array<std::int32_t, 2>, 4> tileIndicesWalking = {}; // [Dir][Animation Frame]
    std::array<std::array<std::int32_t, 2>, 4> tileIndicesPushing = {}; // [Dir][Animation Frame]

    Area* area = nullptr;
    const TileData* tileData = nullptr;
    const SpeedData* speed = nullptr;
};