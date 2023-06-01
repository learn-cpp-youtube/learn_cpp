#pragma once

#include "SoundChannelHandle.h"
#include "SoundHandle.h"
#include "WavHelper.h"
#include <cstdint>
#include <vector>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <SDL.h>

namespace mi
{

class Audio
{
public:
    Audio();
    ~Audio();

    void Init();
    void Free();

    SoundChannelHandle CreateSoundChannel();
    SoundHandle        CreateSound(const std::string& filename);
    SoundHandle        CreateSound(const WavHelper& wavHelper);

private:
    static void CallbackWrapper(void* userData, std::uint8_t* stream, int length);
    void Callback(std::uint8_t *stream, std::uint32_t length);

    void IncSoundChannelRefCount(std::uint32_t channelId);
    void DecSoundChannelRefCount(std::uint32_t channelId);
    void IncSoundRefCount(std::uint32_t soundId);
    void DecSoundRefCount(std::uint32_t soundId);

    bool initialized;
    bool callbackIsGood;
    bool systemIsBigEndian;
    long numChannels;
    long sampleRate;
    std::function<void(double, std::uint8_t*&, std::uint32_t&)> ConvertDouble;
    
    SDL_AudioDeviceID device;
    SDL_AudioSpec     spec;

    // Conversion routines from double to 8/16/32 bit Signed/Unsigned Float/Integer in System/Not
    // System endian.
    static void DoubleTo8UI  (double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo8SI  (double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo16UIS(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo16UIN(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo16SIS(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo16SIN(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo32UIS(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo32UIN(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo32SIS(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleTo32SIN(double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleToFS   (double input, std::uint8_t*& buffer, std::uint32_t& length);
    static void DoubleToFN   (double input, std::uint8_t*& buffer, std::uint32_t& length);

    mutable std::mutex audioMutex;

    class Sound
    {
    public:
        std::vector<float> left;
        std::vector<float> right;

        std::uint32_t refCount = 0;
    };

    class Channel
    {
    public:
        SoundState    soundState;
        SoundEndRule  soundEndRule;
        double        volume;
        std::uint32_t sampleIndex; // Next to be used.
        std::uint32_t soundId;

        std::uint32_t refCount = 0;
    };

    static void Convert(unsigned long targetSampleRate, const WavHelper& wavHelper,
                        Sound* pSound);

    std::unordered_map<std::uint32_t, std::unique_ptr<Channel>> channels;
    std::deque<std::uint32_t> unusedChannelIds;
    std::unordered_map<std::uint32_t, std::unique_ptr<Sound>> sounds;
    std::deque<std::uint32_t> unusedSoundIds;

    friend class SoundChannelHandle;
    friend class SoundHandle;
};

} // End of namespace mi.