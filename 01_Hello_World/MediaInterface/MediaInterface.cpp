#include "MediaInterface.h"
#include "Graphics/BitmapHelper.h"
#include <vector>
#include <chrono>
#include <thread>
#include <map>
#include <utility>
#include <stdexcept>

using error = std::runtime_error;
using std::string;
using std::to_string;
using std::vector;
using std::map;
using std::pair;

namespace
{

bool IsLittleEndian()
{
    uint32_t word = 1;
    uint8_t* byte = reinterpret_cast<uint8_t*>(&word);
    return byte[0];
}

} // End of anonymous namespace.

namespace mi
{

string MediaInterface::SdlGlAttrToString(SDL_GLattr attr)
{
    static const map<SDL_GLattr, string> mapping =
        {
            {SDL_GL_RED_SIZE,                   "SDL_GL_RED_SIZE"},
            {SDL_GL_GREEN_SIZE,                 "SDL_GL_GREEN_SIZE"},
            {SDL_GL_BLUE_SIZE,                  "SDL_GL_BLUE_SIZE"},
            {SDL_GL_ALPHA_SIZE,                 "SDL_GL_ALPHA_SIZE"},
            {SDL_GL_BUFFER_SIZE,                "SDL_GL_BUFFER_SIZE"},
            {SDL_GL_DOUBLEBUFFER,               "SDL_GL_DOUBLEBUFFER"},
            {SDL_GL_DEPTH_SIZE,                 "SDL_GL_DEPTH_SIZE"},
            {SDL_GL_STENCIL_SIZE,               "SDL_GL_STENCIL_SIZE"},
            {SDL_GL_ACCUM_RED_SIZE,             "SDL_GL_ACCUM_RED_SIZE"},
            {SDL_GL_ACCUM_GREEN_SIZE,           "SDL_GL_ACCUM_GREEN_SIZE"},
            {SDL_GL_ACCUM_BLUE_SIZE,            "SDL_GL_ACCUM_BLUE_SIZE"},
            {SDL_GL_ACCUM_ALPHA_SIZE,           "SDL_GL_ACCUM_ALPHA_SIZE"},
            {SDL_GL_STEREO,                     "SDL_GL_STEREO"},
            {SDL_GL_MULTISAMPLEBUFFERS,         "SDL_GL_MULTISAMPLEBUFFERS"},
            {SDL_GL_MULTISAMPLESAMPLES,         "SDL_GL_MULTISAMPLESAMPLES"},
            {SDL_GL_ACCELERATED_VISUAL,         "SDL_GL_ACCELERATED_VISUAL"},
            {SDL_GL_CONTEXT_MAJOR_VERSION,      "SDL_GL_CONTEXT_MAJOR_VERSION"},
            {SDL_GL_CONTEXT_MINOR_VERSION,      "SDL_GL_CONTEXT_MINOR_VERSION"},
            {SDL_GL_CONTEXT_FLAGS,              "SDL_GL_CONTEXT_FLAGS"},
            {SDL_GL_CONTEXT_PROFILE_MASK,       "SDL_GL_CONTEXT_PROFILE_MASK"},
            {SDL_GL_SHARE_WITH_CURRENT_CONTEXT, "SDL_GL_SHARE_WITH_CURRENT_CONTEXT"},
            {SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,   "SDL_GL_FRAMEBUFFER_SRGB_CAPABLE"}
            // Not defined on older versions.
            //{SDL_GL_CONTEXT_RELEASE_BEHAVIOR,   "SDL_GL_CONTEXT_RELEASE_BEHAVIOR"}
        };

    const auto it = mapping.find(attr);
    if (it == mapping.end())
        return "Unknown";

    return it->second;
}

MediaInterface::MediaInterface(
    const std::function<std::unique_ptr<Program>(MediaInterface&)>& programCreator)
{
    window    = nullptr;
    glContext = nullptr;

    Init(programCreator);
    Run();
    Free();

    return;
}

MediaInterface::~MediaInterface()
{
    Free();
    return;
}

void MediaInterface::Free()
{
    // This function should not throw an exception
    // as it is called in the destructor.

    // Do the cleanup in the background.
    if (window)
    {
        SDL_HideWindow(window);

        // Wait a bit for the hide window animation to finish.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup the program.
    program.reset();

    // Cleanup the audio.
    audio.Free();

    // Cleanup the graphics.
    graphics.Free();

    // Cleanup the OpenGL context.
    if (glContext)
        SDL_GL_DeleteContext(glContext);
    glContext = nullptr;

    // Cleanup the window.
    if (window)
        SDL_DestroyWindow(window);
    window = nullptr;

    // Cleanup SDL subsystems (OK to call even if SDL_Init() fails).
    SDL_Quit();

    return;
}

void MediaInterface::Init(
    const std::function<std::unique_ptr<Program>(MediaInterface&)>& programCreator)
{
    // Start up SDL, OpenGL, and create a window.

    // Start by doing a cleanup (just in case).
    Free();

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
        throw error(string("SDL initialization failed: ")+SDL_GetError());

    // Set OpenGL attributes before creating window.
    vector<pair<SDL_GLattr, int>> glAttributes = 
        {
            // Set the OpenGL version to 3.3.
            {SDL_GL_CONTEXT_MAJOR_VERSION, 3},
            {SDL_GL_CONTEXT_MINOR_VERSION, 3},
            // Disallow deprecated functions.
            {SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE},
            // Set the data sizes.
            {SDL_GL_RED_SIZE,              8},
            {SDL_GL_GREEN_SIZE,            8},
            {SDL_GL_BLUE_SIZE,             8},
            {SDL_GL_ALPHA_SIZE,            8},
            {SDL_GL_BUFFER_SIZE,          32},
            // Enable double buffering.
            {SDL_GL_DOUBLEBUFFER,          1}
        };

    SDL_GL_ResetAttributes();
    for (const auto& attr : glAttributes)
    {
        if (SDL_GL_SetAttribute(attr.first, attr.second) < 0)
        {
            throw error("Setting attribute "+SdlGlAttrToString(attr.first)
                +" to "+to_string(attr.second)
                +" failed: "+SDL_GetError());
        }
    }

    // Create a window (100 by 100 pixels).
    uint32_t screenWidth  = 100;
    uint32_t screenHeight = 100;
    window = SDL_CreateWindow("Temporary Title",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              screenWidth,
                              screenHeight,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

    if (!window)
        throw error(string("SDL window creation failed: ")+SDL_GetError());

    // Create an OpenGL context for the window (and make it the current context).
    glContext = SDL_GL_CreateContext(window);

    if (!glContext)
        throw error(string("Creating OpenGL context failed: ")+SDL_GetError());

    // Check the attributes we set made it into the context.
    for (const auto& attr : glAttributes)
    {
        int setValue;
        if (SDL_GL_GetAttribute(attr.first, &setValue) < 0)
        {
            throw error("Getting attribute "+SdlGlAttrToString(attr.first)+" failed: "
                +SDL_GetError());
        }

        if (setValue != attr.second)
        {
            throw error("Attribute "+SdlGlAttrToString(attr.first)
                +" was set to "+to_string(setValue)
                +" not "+to_string(attr.second));
        }
    }

    graphics.Init(screenWidth, screenHeight);

    // Setup the event handler.
    EventHandler::Init();
    eventHandler.events.clear();
    
    // Setup the Audio.
    audio.Init();

    // Setup the elapsed time functionality.
    lastCallOfTimeElapsed = std::chrono::steady_clock::now();

    // Start the user defined program.
    quit = false;
    program = programCreator(*this);

    return;
}

void MediaInterface::Run()
{
    if (quit)
        return;

    SDL_ShowWindow(window);

    while (true)
    {
        eventHandler.Update();
        program->MainLoop();

        if (quit)
            return;
    }
}

SoundChannelHandle MediaInterface::CreateSoundChannel()
{
    return audio.CreateSoundChannel();
}

void MediaInterface::DeleteSoundChannel(SoundChannelHandle& soundChannelHandle)
{
    soundChannelHandle = SoundChannelHandle();
    return;
}

SoundHandle MediaInterface::CreateSound(const string& filename)
{
    return audio.CreateSound(filename);
}

SoundHandle MediaInterface::CreateSound(const WavHelper& wavHelper)
{
    return audio.CreateSound(wavHelper);
}

void MediaInterface::DeleteSound(SoundHandle& soundHandle)
{
    soundHandle = SoundHandle();
    return;
}

ImageHandle MediaInterface::CreateImage(const string& filename, bool smooth)
{
    BitmapHelper bmp;
    bmp.Load(filename);

    uint32_t texture = graphics.CreateTexture(
        bmp.width,
        bmp.height,
        bmp.data.data(),
        smooth);

    return ImageHandle(&graphics, texture);
}

ImageHandle MediaInterface::CreateImage(uint32_t width, uint32_t height, bool smooth)
{
    // Create an empty OpenGL texture object.
    // The format doesn't matter since we are not providing a pointer to the pixel data.
    uint32_t texture = graphics.CreateTexture(width, height, nullptr, smooth);
    return ImageHandle(&graphics, texture);
}

ImageHandle MediaInterface::CreateImage(uint32_t width,
                                        uint32_t height,
                                        const std::uint8_t* data,
                                        bool smooth)
{
    uint32_t texture = graphics.CreateTexture(width, height, data, smooth);
    return ImageHandle(&graphics, texture);
}

void MediaInterface::DeleteImage(ImageHandle& imageHandle)
{
    imageHandle.Free();
    return;
}

void MediaInterface::SaveImage(const ImageHandle& imageHandle, const string& filename)
{
    BitmapHelper bmp;
    bmp.SetDimensions(imageHandle.GetWidth(), imageHandle.GetHeight());
    graphics.GetTexturePixels(imageHandle.texture, bmp.data);

    AdjustColorChannels(imageHandle.GetWidth(), imageHandle.GetHeight(), bmp.data.data());

    bmp.Save(filename);
    return;
}

void MediaInterface::DisplayInWindow(const ImageHandle& imageHandle)
{
    graphics.DisplayTexture(imageHandle.texture);
    //SDL_GL_SetSwapInterval(0); // Gives driver a hint to switch off vsync.
    SDL_GL_SwapWindow(window);
    return;
}

ImageHandle MediaInterface::CreateImage(const ImageHandle& imageHandle,
                                        const Color& keyColor,
                                        bool smooth)
{
    return ImageHandle(&graphics, graphics.AddTransparency(imageHandle.texture, keyColor, smooth));
}

ImageHandle MediaInterface::CreateImage(const ImageHandle& imageHandle,
                                        const ImageHandle& alphaChannel,
                                        bool smooth)
{
    return ImageHandle(&graphics,
        graphics.AddTransparency(imageHandle.texture, alphaChannel.texture, smooth));
}

ImageHandle MediaInterface::CreateImage(const ImageHandle& imageHandle,
                                        const ImageHandle& palette,
                                        uint32_t paletteX, uint32_t paletteY,
                                        uint32_t paletteW, uint32_t paletteH,
                                        bool smooth)
{
    return ImageHandle(&graphics,
        graphics.CreatePalettedTexture(imageHandle.texture, palette.texture,
                                       paletteX, paletteY, paletteW, paletteH, smooth));
}

void MediaInterface::SetWindowSize(long width, long height)
{
    SDL_SetWindowSize(window, width, height);
    graphics.SetScreenSize(width, height);
    return;
}

void MediaInterface::GetDisplaySize(std::uint32_t& width, std::uint32_t& height)
{
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode))
        throw error(string("SDL failed to get display mode: ") + SDL_GetError());

    width  = displayMode.w;
    height = displayMode.h;
    return;
}

void MediaInterface::SetWindowTitle(const string& title)
{
    SDL_SetWindowTitle(window, title.c_str());
    return;
}

void MediaInterface::SetWindowIcon(const ImageHandle& icon)
{
    uint32_t width  = icon.GetWidth();
    uint32_t height = icon.GetHeight();

    vector<uint8_t> pixels;
    graphics.GetTexturePixels(icon.texture, pixels);

    AdjustColorChannels(width, height, pixels.data());
    SetWindowIcon(width, height, pixels.data());
    return;
}

void MediaInterface::SetWindowIcon(const ImageHandle& icon, const ImageHandle& iconAlphaChannel)
{
    uint32_t width  = icon.GetWidth();
    uint32_t height = icon.GetHeight();

    vector<uint8_t> pixels;
    graphics.GetTexturePixels(icon.texture, pixels);

    vector<uint8_t> maskPixels;

    if (iconAlphaChannel.GetWidth()==width && iconAlphaChannel.GetHeight()==height)
        graphics.GetTexturePixels(iconAlphaChannel.texture, maskPixels);
    else
    {
        ImageHandle scaledMask = CreateImage(width, height, iconAlphaChannel.GetSmoothFlag());
        scaledMask.DrawScaledImage(0, 0, width, height,
                                   iconAlphaChannel,
                                   0, 0, iconAlphaChannel.GetWidth(), iconAlphaChannel.GetHeight());

        graphics.GetTexturePixels(scaledMask.texture, maskPixels);
    }

    const int totalPixels = width * height; 
    for (int pixelIndex=0; pixelIndex<totalPixels; ++pixelIndex) 
        pixels[4*pixelIndex+3] = maskPixels[4*pixelIndex];

    SetWindowIcon(width, height, pixels.data());
    return;
}

void MediaInterface::SetWindowIcon(std::uint32_t width, std::uint32_t height, std::uint8_t* data)
{
    SDL_Surface* surface = nullptr;
    try
    {
        if (IsLittleEndian())
        {
            surface = SDL_CreateRGBSurfaceFrom(data, width, height, 32, width*4,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        }
        else
        {
            surface = SDL_CreateRGBSurfaceFrom(data, width, height, 32, width*4,
                0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        }

        if (!surface)
        {
            throw error(string("SDL failed to create a surface for the icon: ")
                +SDL_GetError());
        }

        SDL_SetWindowIcon(window, surface);
        SDL_FreeSurface(surface);
    }
    catch(...)
    {
        SDL_FreeSurface(surface);
        throw;
    }

    return;
}

void MediaInterface::CenterWindow()
{
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    return;
}

void MediaInterface::Close()
{
    quit = true;
    return;    
}

long MediaInterface::TimeElapsed()
{
    using namespace std::chrono;
    auto now = steady_clock::now();

    long elapsedTime = static_cast<long>(
        duration_cast<milliseconds>(now-lastCallOfTimeElapsed).count());

    lastCallOfTimeElapsed = now;
    return elapsedTime;
}

void MediaInterface::RegulateFrames(long framesPerSecond)
{
    // Check if we are looping faster than framesPerSecond.
    // If so, let the CPU sleep for a while.
    long elapsedTime = TimeElapsed();
    long msPerFrame = 1000/framesPerSecond;

    if (elapsedTime < msPerFrame)
        std::this_thread::sleep_for(std::chrono::milliseconds(msPerFrame-elapsedTime));

    TimeElapsed(); // Reset the timer.
    return;
}

void MediaInterface::UpdateImageCache(const ImageHandle* images, size_t size)
{
    if (size == 0 || !images)
        return;

    if (size == 1)
    {
        graphics.UpdateTextureCache(&(images->texture), 1);
        return;
    }

    vector<uint32_t> textures(size);
    for (size_t i=0; i<size; i++)
        textures[i] = images[i].texture;

    graphics.UpdateTextureCache(textures.data(), size);
    return;
}

void MediaInterface::ClearImageCache()
{
    graphics.ClearTextureCache();
    return;
}

void MediaInterface::AdjustColorChannels(std::uint32_t width,
                                         std::uint32_t height,
                                         std::uint8_t* data)
{
    const int totalPixels = width * height; 
    for (int pixelIndex=0; pixelIndex<totalPixels; ++pixelIndex) 
    {
        std::uint8_t alpha = data[4*pixelIndex+3];
        if (alpha == 0)
            continue;

        for (int i=0; i<3; ++i)
            data[4*pixelIndex+i] = std::min(data[4*pixelIndex+i]*255/alpha, 255);
    }
}

} // End of namespace mi.
