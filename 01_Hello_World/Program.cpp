#include "Program.h"
#include <iostream>

Program::Program(mi::MediaInterface* mi)
{
    // Store the MediaInterface object so that we can use it later in other functions.
    this->mi = mi;

    Reload();

    mi->CenterWindow();
    mi->SetWindowTitle("Demo - Learn C++");
    mi->SetWindowIcon(mi->CreateImage("Media/icon.bmp"),
                      mi->CreateImage("Media/icon_mask.bmp"));

    return;
}

void Program::Reload()
{
    json::Object metadata = json::ParseFile("Data/Metadata.json").GetObject();
    tileData.Init(*mi, metadata);
    speedData = SpeedData{fps, tileData};
    areaData.Init(speedData, tileData, metadata);

    areaIndex = areaData.GetAreaIndex("Test");

    tileSizeInPixels = tileData.GetTileSizeInPixels();
    backgroundWidthInTiles  = areaData.GetArea(areaIndex).widthInTiles;
    backgroundHeightInTiles = areaData.GetArea(areaIndex).heightInTiles;;

    // Set the size of the image which represents the window.
    std::int32_t screenWidth  = scale * tileSizeInPixels * backgroundWidthInTiles;
    std::int32_t screenHeight = scale * tileSizeInPixels * backgroundHeightInTiles;

    // Set the window dimensions.
    mi->SetWindowSize(screenWidth, screenHeight);

    // "screen" is the image which represents what we'll see in the window.
    screen = mi->CreateImage(tileSizeInPixels * backgroundWidthInTiles,
                             tileSizeInPixels * backgroundHeightInTiles);

    // Initialize key states (and display helper).
    keyStates = KeyStates{};
    keyStatesDisplayHelper = KeyStatesDisplayHelper{*mi, tileData};

    // Initialize hero.
    hero = Hero{speedData, tileData, areaData.GetArea(areaIndex)};
}

void Program::MainLoop()
{
    HandleInput();
    Display();
    return;
}

void Program::HandleInput()
{
    // Check if there are pending events that need to be dealt with.
    while (!mi->GetEvents().empty())
    {
        // Get the event and remove it from the list of pending events.
        mi::Event e = mi->GetEvents().front();
        mi->GetEvents().pop_front();

        // Check if the event is that the user clicked the button to close the window
        // or the user pressed the Q key
        // or the user pressed the Escape key.
        if (e.type == mi::EventType::QUIT
            || (e.type == mi::EventType::KEY_DOWN && e.key == mi::Key::Q)
            || (e.type == mi::EventType::KEY_DOWN && e.key == mi::Key::ESCAPE))
        {
            mi->Close();
            continue;
        }

        // Check if we should toogle displaying the keys pressed.
        if (e.type == mi::EventType::KEY_DOWN && e.key == mi::Key::F1)
        {
            displayKeysPressed = !displayKeysPressed;
            continue;
        }

        // Check if we should reload the tiles and metadata.
        if (e.type == mi::EventType::KEY_DOWN && e.key == mi::Key::F5)
        {
            Reload();
            continue;
        }

        // Update the keyStates.
        if (e.type == mi::EventType::KEY_DOWN || e.type == mi::EventType::KEY_UP)
        {
            if (e.key == mi::Key::UP)
                keyStates.upPressed    = (e.type==mi::EventType::KEY_DOWN);

            if (e.key == mi::Key::RIGHT)
                keyStates.rightPressed = (e.type==mi::EventType::KEY_DOWN);

            if (e.key == mi::Key::DOWN)
                keyStates.downPressed  = (e.type==mi::EventType::KEY_DOWN);

            if (e.key == mi::Key::LEFT)
                keyStates.leftPressed  = (e.type==mi::EventType::KEY_DOWN);
        }

        // Any events we haven't explicitly handled will be ignored.
    }

    return;
}

void Program::Display()
{
    hero.Update(keyStates);

    Area& area = areaData.GetArea(areaIndex);
    area.Update();

    // Draw the area.
    for (std::int32_t y=0; y<backgroundHeightInTiles; ++y)
    for (std::int32_t x=0; x<backgroundWidthInTiles;  ++x)
    {
        std::int32_t tileIndex = area.background.GetTileIndex(x, y);
        if (tileIndex != TileData::NotFound)
            tileData.DrawTile(screen, x * tileSizeInPixels, y * tileSizeInPixels, tileIndex);
    }

    for (const auto& obj : area.foreground.objects)
    {
        if (obj.tile != TileData::NotFound)
            tileData.DrawTile(screen, obj.x, obj.y, obj.tile);
    }

    // Draw the hero.
    tileData.DrawTile(screen, hero.GetXPos(), hero.GetYPos(), hero.GetTileIndex());

    // Draw the keys being pressed.
    if (displayKeysPressed)
    {
        mi::ImageHandle& img = keyStatesDisplayHelper.GetImage(keyStates); 
        screen.DrawImage(
            screen.GetWidth() - img.GetWidth() - tileSizeInPixels/4,
            screen.GetHeight() - img.GetHeight() - tileSizeInPixels/4,
            img,
            0, 0, img.GetWidth(), img.GetHeight(),
            200);
    }

    // If we are running too fast, slow down the program.
    mi->RegulateFrames(fps);

    // Display in the window.
    mi->DisplayInWindow(screen);
    return;
}
