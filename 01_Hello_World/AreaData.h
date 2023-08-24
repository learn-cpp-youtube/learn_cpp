#pragma once

#include "MediaInterface/MediaInterface.h"
#include "Utilities/Json.h"
#include "TileData.h"
#include "SpeedData.h"
#include <cstdint>
#include <map>
#include <vector>

struct AlignedArea
{
    std::vector<std::vector<std::int32_t>> tiles;

    std::int32_t  GetTileIndex(std::int32_t x, std::int32_t y) const { return tiles[y][x]; }
    std::int32_t& GetTileIndex(std::int32_t x, std::int32_t y)       { return tiles[y][x]; }
};

enum class BlockType : std::int8_t
{
    Immovable,
    SinglePushGridAligned,
    MultiplePushGridAligned,
    Unaligned
};

struct BlockProperties
{
    BlockType type       = BlockType::Immovable;
    bool isMoving        = false;
    bool finishedMoving  = true; // Only used by SinglePushGridAligned.
    std::int32_t targetX = 0;
    std::int32_t targetY = 0;
    std::int32_t offsetX = 0;
    std::int32_t offsetY = 0;
    std::int32_t animMoveTimeMicrosec = 0;

    std::int32_t glanceDist = 0; // In pixels.
    std::int32_t pushPeriodMicrosec = 0;
    std::int32_t timeToMoveAPixelOrthogonallyMicrosec = 0; 
};

struct Object
{
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t tile = TileData::NotFound;

    BlockProperties blockProperties;
};

struct DynamicArea
{
    std::vector<Object> objects;
};

struct Area
{
    Area(std::int32_t widthInTiles, std::int32_t heightInTiles, const SpeedData& speedData,
         const TileData& tileData);
    void Update();

    std::int32_t widthInTiles = 0;
    std::int32_t heightInTiles = 0;
    AlignedArea  background;
    DynamicArea  foreground;

    const SpeedData* speed = nullptr;
};

class AreaData
{
public:
    static const std::int32_t NotFound = -1;

    AreaData() {}
    ~AreaData() { Free(); }

    void Init(const SpeedData& speedData, const TileData& tiledata, const json::Object& metadata);
    void Free();

    // Returns AreaData::NotFound or the index of the area.
    std::int32_t GetAreaIndex(const std::string& id) const;

    Area&       GetArea(std::int32_t areaIndex);
    const Area& GetArea(std::int32_t areaIndex) const;

private:
    std::vector<Area> areas;
    std::map<std::string, std::int32_t> stringToAreaIndex;
};
