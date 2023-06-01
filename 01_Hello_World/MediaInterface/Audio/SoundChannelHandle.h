#pragma once

#include <cstdint>

namespace mi
{

//Forward class declarations.
class Audio;
class SoundHandle;

enum class SoundState: std::uint8_t
{
    Stopped,
    Playing
};

enum class SoundEndRule: std::uint8_t
{
    Loop,
    Stop,
    StopAndDeleteChannel
};

class SoundChannelHandle
{
public:
    SoundChannelHandle() noexcept;
    ~SoundChannelHandle();

    SoundChannelHandle(const SoundChannelHandle& channel) noexcept;
    SoundChannelHandle(SoundChannelHandle&& channel) noexcept;

    SoundChannelHandle& operator=(const SoundChannelHandle& channel) noexcept;
    SoundChannelHandle& operator=(SoundChannelHandle&& channel) noexcept;

    bool operator==(const SoundChannelHandle& rhs) const noexcept;
    bool operator!=(const SoundChannelHandle& rhs) const noexcept { return !(*this == rhs); }

    // Loads the sound in stopped state.
    // The sound can be SoundHandle{} to allow for unsetting of a sound.
    void Load(const SoundHandle& sound,
              SoundEndRule soundEndRule = SoundEndRule::Stop,
              double volume = 1.0,
              std::uint32_t sampleIndex = 0);

    void LoadAndPlay(const SoundHandle& sound,
                     SoundEndRule soundEndRule = SoundEndRule::Stop,
                     double volume = 1.0,
                     std::uint32_t sampleIndex = 0);

    void Play();
    void Pause();
    void Stop();

    void GetState(SoundState& state, std::uint32_t& sampleIndex);
    bool IsPlaying();

private:
    SoundChannelHandle(Audio* audio, std::uint32_t channelId);

    void SetSoundChannel(Audio* audio, std::uint32_t channelId);
    void Free();

    std::uint32_t channelId;
    Audio* audio;

    friend class Audio;
};

} // End of namespace mi.