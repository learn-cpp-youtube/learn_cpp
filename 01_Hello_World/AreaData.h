#pragma once

#include "MediaInterface/MediaInterface.h"
#include "Utilities/Json.h"
#include "TileData.h"
#include <cstdint>
#include <map>
#include <vector>

struct Area
{
    std::int32_t widthInTiles = 0;
    std::int32_t heightInTiles = 0;
    std::vector<std::vector<std::int32_t>> backgroundTiles;
    std::vector<std::vector<std::int32_t>> foregroundTiles;

    std::int32_t  GetForegroundTileIndex(std::int32_t x, std::int32_t y) const;
    std::int32_t& GetForegroundTileIndex(std::int32_t x, std::int32_t y);
    std::int32_t  GetBackgroundTileIndex(std::int32_t x, std::int32_t y) const;
    std::int32_t& GetBackgroundTileIndex(std::int32_t x, std::int32_t y);
};

class AreaData
{
public:
    static const std::int32_t NotFound = -1;

    AreaData() {}
    ~AreaData() { Free(); }

    void Init(const TileData& tiledata, const json::Object& metadata);
    void Free();

    // Returns AreaData::NotFound or the index of the area.
    std::int32_t GetAreaIndex(const std::string& id) const;

    Area&       GetArea(std::int32_t areaIndex);
    const Area& GetArea(std::int32_t areaIndex) const;

private:
    std::vector<Area> areas;
    std::map<std::string, std::int32_t> stringToAreaIndex;
};
