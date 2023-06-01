// Comments begin with two slashes and last till the end of the line.
// They are ignored by the compiler.
// They serve as handy notes to people reading the code.

// The execution of the program is conceptually:
// 1. Program(MediaInterface& mi) - Initialization.
// 2. MainLoop()                  - Handle input, display graphics, etc.
// 3. ~Program()                  - Cleanup then terminate.

// "#include" tells the compiler to insert the contents of some files here
// (they have declarations the compiler needs).
#include "MediaInterface/MediaInterface.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>

// List of all the parts of the MediaInterface we will be using.
using mi::MediaInterface;
using mi::Color;
using mi::ImageHandle;
using mi::SoundHandle;
using mi::SoundChannelHandle;
using mi::SoundState;
using mi::SoundEndRule;
using mi::Event;
using mi::EventType;
using mi::Key;
using mi::MouseButton;

// "class Program" is a custom type that represents what we want to run using the MediaInterface
// framework.
// The execution of the program is conceptually:
// 1. Program(MediaInterface& mi) - Initialization.
// 2. MainLoop()                  - Handle input, display graphics, etc.
// 3. ~Program()                  - Cleanup then terminate.
class Program : public mi::Program
{
public:
    // These 2 functions are necessary (otherwise we'll get compilation errors).
    explicit Program(MediaInterface& mediaInterface);
    void MainLoop() override;

    // This function can be omitted (the compiler will auto generate one if it's missing). 
    ~Program();

private:
    // Functions that our program uses.
    void HandleInput();
    void Display();

    // Store data we want to remember and persist between function calls.
    MediaInterface* mi;
    long screenWidth;
    long screenHeight;
    long targetFPS;
    long numFrames;

    SoundHandle sound;

    ImageHandle screen;
    ImageHandle front;
    ImageHandle back;
};

Program::Program(MediaInterface& mediaInterface)
{
    // This function gets called (automatically) when the program starts.
    // Do any loading and initialization here.

    // The MediaInterface object manages the window, graphics, audio, and user interactions.
    // Our Program is given access to it when it is created.

    // Store the MediaInterface object so that we can use it later in other functions.
    mi = &mediaInterface;

    // Set the window dimensions.
    screenWidth  = 640;
    screenHeight = 480;
    mi->SetWindowSize(screenWidth, screenHeight);

    // Move the window into the center of the screen.
    mi->CenterWindow();

    // Set the title.
    mi->SetWindowTitle("Hello World - Learn C++");

    // Set the icon.
    mi->SetWindowIcon(mi->CreateImage("Media/icon.bmp"),
                      mi->CreateImage("Media/icon_mask.bmp"));

    // Decide on the frames per second we want to display animation at.
    targetFPS = 60;

    // Load a sound (which we'll play later).
    sound = mi->CreateSound("Media/sound.wav");

    // Create some images which we'll use in a simple animation.

    // "screen" is the image which represents what we'll see in the window.
    screen = mi->CreateImage(screenWidth, screenHeight);

    // "back" is an image of some stars which will be the background of the animation.
    back = mi->CreateImage("Media/background.bmp");
    
    // Create an image which is just a solid plain color.
    ImageHandle blank = mi->CreateImage(screenWidth, screenHeight);
    blank.Clear(Color(255,255,255)); // Color(255,255,255) is white.

    // Load an image which will indicate transparency
    // white pixels = opaque,
    // black pixels = transparent.
    ImageHandle opacity = mi->CreateImage("Media/opacity.bmp", true);

    // "front" is an image which is a solid color with some parts made transparent.
    front = mi->CreateImage(blank, opacity);

    // Keep track of the number of frames that we've displayed, in order to do the animation.
    numFrames = 0;

    // Finished initialization. 
    return;
}

void Program::MainLoop()
{
    // This function gets called (automatically) over and over again, until we decide to quit by
    // calling mi->Close(). At the very least this function should handle the event queue given by
    // mi->GetEvents().

    // To keep things organized we split the work into two functions HandleInput() and Display().
    HandleInput();
    Display();

    return;
}

Program::~Program()
{
    // This function gets called (automatically) when the program ends.
    // Do not try to call this function directly instead use mi->Close().
    // Do any saving of data and cleaning up here.

    // In this particular program we have nothing to clean up (it is all done for us automatically).
    return;
}

void Program::HandleInput()
{
    // Handle user input here.
    // This function gets called once per frame.

    // Check if there are pending events that need to be dealt with.
    while (!mi->GetEvents().empty())
    {
        // Get the event and remove it from the list of pending events.
        Event e = mi->GetEvents().front();
        mi->GetEvents().pop_front();

        // Check if the event is that the user clicked the button to close the window
        // or the user pressed the Q key
        // or the user pressed the Escape key.
        if (e.type == EventType::QUIT
            || (e.type == EventType::KEY_DOWN && e.key == Key::Q)
            || (e.type == EventType::KEY_DOWN && e.key == Key::ESCAPE))
        {
            // Display a message to the console.
            std::cout << "Goodbye World." << std::endl;

            // Tell the MediaInterface object we want to terminate the program.
            mi->Close();
            continue;
        }

        // Check if the event is that the user left clicked.
        if (e.type == EventType::MOUSE_DOWN && e.button == MouseButton::LEFT)
        {
            // Play the sound we loaded in Program().
            mi->CreateSoundChannel().LoadAndPlay(sound, SoundEndRule::StopAndDeleteChannel);
            continue;
        }

        // Any events we haven't explicitly handled will be ignored.
    }

    return;
}

void Program::Display()
{
    // Draw graphics to the screen.

    // Update the frame count.
    // We'll use it to calculate the opacity for the transition animation.
    numFrames++;

    // numFrames starts off as zero and increases,
    // which means opacity starts off negative and increases.
    long opacity = 5*(numFrames-50);

    // Opacities should be between 0 and 255, so we force that here.
    opacity = std::clamp<long>(opacity, 0, 255);

    // The overall effect is that the opacity is zero for a while,
    // then steadily increases to 255,
    // and then stays at 255.

    // Draw the background.
    screen.DrawScaledImage(0, 0, screen.GetWidth(), screen.GetHeight(),
                           back, 0, 0, back.GetWidth(), back.GetHeight());


    // Draw the foreground.
    screen.DrawScaledImage(0, 0, screen.GetWidth(), screen.GetHeight(),
                           front, 0, 0, front.GetWidth(), front.GetHeight(),
                           opacity);

    // If we are running too fast, slow down the program.
    mi->RegulateFrames(targetFPS);

    // Display in the window.
    mi->DisplayInWindow(screen);

    return;
}

// The main function is the starting point of every C++ program.
int main(int argc, char** argv)
{
    // "try" and "catch" are used to stop certain errors crashing our application, instead they get
    // caught displayed to the user in the terminal before everything is gracefully shutdown.
    try
    {
        // Create a MediaInterface object which will initialize the graphics library, create a
        // window, and make a Program object (according to the specifications given above), then
        // call its MainLoop function repeatedly. 
        MediaInterface([](MediaInterface& mi){ return std::make_unique<Program>(mi); });
    }
    catch(const std::exception& e)
    {
        std::cout << "\nERROR: Exception occurred: " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "\nERROR: Unknown exception occurred." << std::endl;
    }

    return 0;
}




