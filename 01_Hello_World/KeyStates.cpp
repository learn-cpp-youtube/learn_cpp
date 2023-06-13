#include "KeyStates.h"

KeyStatesDisplayHelper::KeyStatesDisplayHelper(mi::MediaInterface& mi, const TileData& tileData)
{
    this->tileData = &tileData;

    tileSizeInPixels = tileData.GetTileSizeInPixels();
    spacingBetweenTilesInPixels = tileSizeInPixels / 8;

    upTiles[0]    = tileData.GetTileIndex("TestTiles/KeyUpArrowUnpushed");
    upTiles[1]    = tileData.GetTileIndex("TestTiles/KeyUpArrowPushed");
    downTiles[0]  = tileData.GetTileIndex("TestTiles/KeyDownArrowUnpushed");
    downTiles[1]  = tileData.GetTileIndex("TestTiles/KeyDownArrowPushed");
    leftTiles[0]  = tileData.GetTileIndex("TestTiles/KeyLeftArrowUnpushed");
    leftTiles[1]  = tileData.GetTileIndex("TestTiles/KeyLeftArrowPushed");
    rightTiles[0] = tileData.GetTileIndex("TestTiles/KeyRightArrowUnpushed");
    rightTiles[1] = tileData.GetTileIndex("TestTiles/KeyRightArrowPushed");

    img = mi.CreateImage(3*tileSizeInPixels + 4*spacingBetweenTilesInPixels,
                         2*tileSizeInPixels + 3*spacingBetweenTilesInPixels);
}

mi::ImageHandle& KeyStatesDisplayHelper::GetImage(const KeyStates& keyStates)
{
    img.Clear({255,255,255,0});

    // Up key.
    tileData->DrawTile(img,
                       tileSizeInPixels + 2*spacingBetweenTilesInPixels,
                       spacingBetweenTilesInPixels,
                       upTiles[keyStates.upPressed]);

    // Down key.
    tileData->DrawTile(img,
                       tileSizeInPixels + 2*spacingBetweenTilesInPixels,
                       tileSizeInPixels + 2*spacingBetweenTilesInPixels,
                       downTiles[keyStates.downPressed]);

    // Left key.
    tileData->DrawTile(img,
                       spacingBetweenTilesInPixels,
                       tileSizeInPixels + 2*spacingBetweenTilesInPixels,
                       leftTiles[keyStates.leftPressed]);

    // Right key.
    tileData->DrawTile(img,
                       2*tileSizeInPixels + 3*spacingBetweenTilesInPixels,
                       tileSizeInPixels + 2*spacingBetweenTilesInPixels,
                       rightTiles[keyStates.rightPressed]);

    return img;
}