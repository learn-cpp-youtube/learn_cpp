#pragma once

#include <SDL.h>
#include "Graphics/Color.h"
#include "Graphics/ImageHandle.h"
#include "Graphics/Graphics.h"
#include "Events/Event.h"
#include "Audio/Audio.h"
#include "Audio/SoundChannelHandle.h"
#include "Audio/SoundHandle.h"
#include "Audio/WavHelper.h"
#include <string>
#include <cstdint>
#include <chrono>
#include <functional>
#include <memory>

namespace mi
{

class Program
{
public:
    virtual ~Program() = default;
    virtual void MainLoop() = 0;
};

class MediaInterface
{
public:
    explicit MediaInterface(
        const std::function<std::unique_ptr<Program>(MediaInterface&)>& programCreator);
    ~MediaInterface();

    // Currently can only load .wav files (16-bit PCM).
    SoundHandle        CreateSound(const std::string& filename);
    SoundHandle        CreateSound(const WavHelper& wavHelper);
    void               DeleteSound(SoundHandle& soundHandle);
    SoundChannelHandle CreateSoundChannel();
    void               DeleteSoundChannel(SoundChannelHandle& soundChannelHandle);
    
    // Currently can only load .bmp files.
    ImageHandle CreateImage(const std::string& filename, bool smooth=false);
    ImageHandle CreateImage(std::uint32_t width, std::uint32_t height, bool smooth=false);
    ImageHandle CreateImage(std::uint32_t width,
                            std::uint32_t height,
                            const std::uint8_t* data, // RGBA
                            bool smooth = false);
    ImageHandle CreateImage(const ImageHandle& imageHandle,
                            const Color& keyColor,
                            bool smooth=false);
    ImageHandle CreateImage(const ImageHandle& imageHandle,
                            const ImageHandle& alphaChannel,
                            bool smooth=false);
    // This function is slow, consider prerendering where possible.
    ImageHandle CreateImage(const ImageHandle& imageHandle,
                            const ImageHandle& palette,
                            std::uint32_t paletteX, std::uint32_t paletteY,
                            std::uint32_t paletteW, std::uint32_t paletteH,
                            bool smooth=false);
    void DeleteImage(ImageHandle& imageHandle);
    void DisplayInWindow(const ImageHandle& imageHandle);
    void SaveImage(const ImageHandle& imageHandle, const std::string& filename);

    void GetDisplaySize(std::uint32_t& width, std::uint32_t& height);
    void SetWindowSize(long width, long height);
    void SetWindowTitle(const std::string& title);
    void SetWindowIcon(const ImageHandle& icon);
    // When SDL downsamples images it can overly darken semi-transparent pixels, this can be fixed
    // by providing appropriate color values to the color channels of the transparent pixels.
    // Due to the way semi-transparent images are internally stored (i.e. color channels are
    // premultiplied by the alpha channel) we can't use a single image but instead need to pass the
    // color and alpha channels separately.
    void SetWindowIcon(const ImageHandle& icon, const ImageHandle& iconAlphaChannel);
    void CenterWindow();
    void Close();

    // Returns the time in milliseconds since the
    // last time this function was called.
    long TimeElapsed();
    void RegulateFrames(long framesPerSecond = 60);

    std::deque<Event>& GetEvents() { return eventHandler.events; }
    std::string KeyToString(Key key) { return EventHandler::KeyToString(key); }
    std::string ButtonToString(MouseButton button) { return EventHandler::ButtonToString(button); }
    void GetMouseState(std::int32_t& buttonsPressed, std::int32_t& x, std::int32_t& y)
        { EventHandler::GetMouseState(buttonsPressed, x, y); }

    // Used to speed up drawing by manually providing hints to the graphics object.
    void UpdateImageCache(const ImageHandle* images, size_t size);
    void ClearImageCache();

private:
    static std::string SdlGlAttrToString(SDL_GLattr attr);

    void Init(const std::function<std::unique_ptr<Program>(MediaInterface&)>& programCreator);
    void Free();
    void Run();

    // The color channels are normally premultiplied by the alpha channel, to make compositing
    // semi-transparent images quicker. This function undoes that.
    void AdjustColorChannels(std::uint32_t width, std::uint32_t height, std::uint8_t* data);

    void SetWindowIcon(std::uint32_t width, std::uint32_t height, std::uint8_t* data);


    std::unique_ptr<Program> program;
    SDL_Window*   window;
    SDL_GLContext glContext;
    Audio         audio;
    Graphics      graphics;
    EventHandler  eventHandler;

    bool quit;

    std::chrono::time_point<std::chrono::steady_clock> lastCallOfTimeElapsed;

    friend class ImageHandle;

    struct DeleterHelper
    {
        MediaInterface* mi = nullptr;
        ~DeleterHelper() { if (mi) mi->Free(); }
    };

    // Important that this is the last member (so it is the first to get deleted).
    DeleterHelper deleter;
};

} // End of namespace mi.
