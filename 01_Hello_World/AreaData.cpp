#include "AreaData.h"
#include "JsonFileHelper.h"
#include <stdexcept>

using error = std::runtime_error;

Area::Area(std::int32_t widthInTiles, std::int32_t heightInTiles, const SpeedData& speedData,
           const TileData& tileData)
{
    speed = &speedData;

    this->widthInTiles  = widthInTiles;
    this->heightInTiles = heightInTiles;

    // Resize tile arrays.
    background.tiles.resize(heightInTiles);
    for (std::vector<std::int32_t>& v : background.tiles)
        v.resize(widthInTiles, TileData::NotFound);

    foreground.objects.resize(0);
}

void Area::Update()
{
    for (auto& obj : foreground.objects)
    {
        BlockProperties& block = obj.blockProperties;

        if (block.type == BlockType::Unaligned)
        {
            if (block.animMoveTimeMicrosec < speed->frameIntervalMicrosec)
                block.animMoveTimeMicrosec = 0;
            else
                block.animMoveTimeMicrosec -= speed->frameIntervalMicrosec;
            continue;
        }

        if (block.type != BlockType::SinglePushGridAligned &&
            block.type != BlockType::MultiplePushGridAligned)
            continue;

        while (block.animMoveTimeMicrosec < speed->frameIntervalMicrosec)
        {
            if (!block.isMoving)
                block.animMoveTimeMicrosec = speed->frameIntervalMicrosec;
            else
            {
                if (obj.x == block.targetX && obj.y == block.targetY)
                {
                    block.isMoving = false;
                    if (block.type == BlockType::SinglePushGridAligned)
                        block.finishedMoving = true;
                    continue;
                }

                block.animMoveTimeMicrosec += block.timeToMoveAPixelOrthogonallyMicrosec;

                obj.x += block.offsetX;
                obj.y += block.offsetY;
            }
        }

        // Do the rest of the waiting in the next frame.
        block.animMoveTimeMicrosec -= speed->frameIntervalMicrosec;
    }
}

void AreaData::Init(const SpeedData& speedData,
                    const TileData& tileData,
                    const json::Object& metadata)
{
    Free();

    const std::int32_t tileSize = tileData.GetTileSizeInPixels();

    // Read the areas info.
    const json::Object& areasObj = GetValue(metadata, "Areas").GetObject();

    // Read the area info.
    for (const auto& [name, data] : areasObj)
    {
        // Map name to area index.
        std::int32_t areaIndex = static_cast<std::int32_t>(areas.size());
        stringToAreaIndex.insert({name, static_cast<std::int32_t>(areas.size())});

        // Get dimension info.
        const json::Object& dataObj = data.GetObject();
        std::int32_t widthInTiles  = static_cast<std::int32_t>(
                                     GetValue(dataObj, "WidthInTiles").GetInteger());
        std::int32_t heightInTiles = static_cast<std::int32_t>(
                                     GetValue(dataObj, "HeightInTiles").GetInteger());

        // Add new area to areas array.
        areas.emplace_back(Area{widthInTiles, heightInTiles, speedData, tileData});
        Area& area = areas.back();

        // Create character to tile index map.
        std::map<char, std::int32_t> charToTile;

        const json::Object& mappingObj = GetValue(dataObj, "TileMapping").GetObject();
        for (const auto& [tileChar, tileId] : mappingObj)
        {
            if (tileChar.size() != 1)
                throw error("Expecting a single character for tile mapping.");

            std::int32_t tileIndex = tileData.GetTileIndex(tileId.GetString());
            charToTile.insert({tileChar[0], tileIndex});
        }

        // Read background tiles.
        const json::Array& backgroundTilesArray = GetValue(dataObj, "Background").GetArray();
        if (backgroundTilesArray.size() != area.heightInTiles)
            throw error("Tile array height doesn't match HeightInTiles field.");

        for (std::int32_t y=0; y<area.heightInTiles; ++y)
        {
            std::string row = backgroundTilesArray[y].GetString();

            if (row.size() != area.widthInTiles)
                throw error("Tile array width doesn't match WidthInTiles field.");

            for (std::int32_t x=0; x<area.widthInTiles; ++x)
            {
                auto it = charToTile.find(row[x]);
                if (it != charToTile.end())
                    area.background.tiles[y][x] = it->second;
            }
        }

        // Read foreground tiles.
        const json::Array& foregroundTilesArray = GetValue(dataObj, "Foreground").GetArray();
        if (foregroundTilesArray.size() != area.heightInTiles)
            throw error("Tile array height doesn't match HeightInTiles field.");

        for (std::int32_t y=0; y<area.heightInTiles; ++y)
        {
            std::string row = foregroundTilesArray[y].GetString();

            if (row.size() != area.widthInTiles)
                throw error("Tile array width doesn't match WidthInTiles field.");

            for (std::int32_t x=0; x<area.widthInTiles; ++x)
            {
                auto it = charToTile.find(row[x]);
                if (it != charToTile.end())
                    area.foreground.objects.emplace_back(x*tileSize, y*tileSize, it->second);
            }
        }

        // Fill in extra data about object tiles.
        std::int32_t pushBlockId         = tileData.GetTileIndex("TestTiles/BlockPushable");
        std::int32_t singlePushBlockId   = tileData.GetTileIndex("TestTiles/BlockSinglePush");
        std::int32_t multiplePushBlockId = tileData.GetTileIndex("TestTiles/BlockMultiplePush");
        for (auto& obj : area.foreground.objects)
        {
            auto& block = obj.blockProperties;
            block.finishedMoving = false;

            if (obj.tile == pushBlockId)
                block.type = BlockType::Unaligned;
            else
            if (obj.tile == singlePushBlockId)
                block.type = BlockType::SinglePushGridAligned;
            else
            if (obj.tile == multiplePushBlockId)
                block.type = BlockType::MultiplePushGridAligned;
            else
            {
                block.type = BlockType::Immovable;
                block.finishedMoving = true;
            }

            const auto& tileMetadata = tileData.GetTileMetadata(obj.tile);
            block.glanceDist = tileMetadata.glanceDist;
            block.pushPeriodMicrosec = tileMetadata.pushPeriod * 1000 * speedData.slowDownFactor;
            block.timeToMoveAPixelOrthogonallyMicrosec = 
                tileMetadata.pushingSpeedPeriod * 1000 / tileData.GetTileSizeInPixels()
                * speedData.slowDownFactor;
        }
    }
}

void AreaData::Free()
{
    areas.clear();
    stringToAreaIndex.clear();
}

std::int32_t AreaData::GetAreaIndex(const std::string& id) const
{
    auto it = stringToAreaIndex.find(id);
    return (it == stringToAreaIndex.end()) ? NotFound : it->second;
}

Area& AreaData::GetArea(std::int32_t areaIndex)
{
    return areas[areaIndex];
}

const Area& AreaData::GetArea(std::int32_t areaIndex) const
{
    return areas[areaIndex];
}