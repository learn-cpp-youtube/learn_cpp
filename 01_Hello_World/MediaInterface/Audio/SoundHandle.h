#pragma once

#include <cstdint>

namespace mi
{

//Forward class declarations.
class Audio;
class SoundChannelHandle;

class SoundHandle
{
public:
    SoundHandle() noexcept;
    ~SoundHandle();

    SoundHandle(const SoundHandle& snd) noexcept;
    SoundHandle(SoundHandle&& snd) noexcept;

    SoundHandle& operator=(const SoundHandle& snd) noexcept;
    SoundHandle& operator=(SoundHandle&& snd) noexcept;

    bool operator==(const SoundHandle& rhs) const noexcept;
    bool operator!=(const SoundHandle& rhs) const noexcept { return !(*this == rhs); }

private:
    SoundHandle(Audio* audio, std::uint32_t soundId);

    void SetSound(Audio* audio, std::uint32_t soundId);
    void Free();

    std::uint32_t soundId;
    Audio*        audio;

    friend class Audio;
    friend class SoundChannelHandle;
};

} // End of namespace mi.