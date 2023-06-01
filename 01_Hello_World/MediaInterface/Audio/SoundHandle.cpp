#include "SoundHandle.h"
#include "Audio.h"
#include <cstdint>

using std::uint32_t;

namespace mi
{

SoundHandle::SoundHandle() noexcept
{
    SetSound(nullptr, 0);
    return;
}

SoundHandle::SoundHandle(Audio* audio, uint32_t soundId)
{
    SetSound(audio, soundId);
    return;
}

SoundHandle::~SoundHandle()
{
    try
    {
        Free();
    }
    catch(...)
    {
    }
}

SoundHandle::SoundHandle(const SoundHandle& snd) noexcept
{
    soundId = snd.soundId;
    audio   = snd.audio;

    if (audio)
        audio->IncSoundRefCount(soundId);

    return;
}

SoundHandle::SoundHandle(SoundHandle&& snd) noexcept
{
    soundId = snd.soundId;
    audio   = snd.audio;

    snd.soundId = 0;
    snd.audio   = nullptr;

    return;
}

SoundHandle& SoundHandle::operator=(const SoundHandle& snd) noexcept
{
    if (&snd == this)
        return *this;

    Free();

    soundId = snd.soundId;
    audio   = snd.audio;
    
    if (audio)
        audio->IncSoundRefCount(soundId);

    return *this;
}

SoundHandle& SoundHandle::operator=(SoundHandle&& snd) noexcept
{
    if (&snd == this)
        return *this;

    Free();

    soundId = snd.soundId;
    audio   = snd.audio;

    snd.soundId = 0;
    snd.audio   = nullptr;

    return *this;
}

bool SoundHandle::operator==(const SoundHandle& rhs) const noexcept
{
    if ((audio==nullptr || soundId==0) && (rhs.audio==nullptr || rhs.soundId==0))
        return true;

    return audio==rhs.audio && soundId==rhs.soundId;
}

void SoundHandle::SetSound(Audio* audio, uint32_t soundId)
{
    if(!audio || soundId==0)
    {
        this->soundId = 0;
        this->audio   = nullptr;
    }
    else
    {        
        this->soundId = soundId;
        this->audio   = audio;
        audio->IncSoundRefCount(soundId);
    }

    return;
}

void SoundHandle::Free()
{
    if (audio)
        audio->DecSoundRefCount(soundId);

    soundId = 0;
    audio   = nullptr;
    return;
}

} // End of namespace mi.