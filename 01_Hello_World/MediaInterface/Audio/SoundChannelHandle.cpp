#include "SoundChannelHandle.h"
#include "Audio.h"
#include <stdexcept>
#include <mutex>
#include <cstdint>

using error = std::runtime_error;
using std::mutex;
using std::lock_guard;
using std::uint32_t;

namespace mi
{

SoundChannelHandle::SoundChannelHandle() noexcept
{
    SetSoundChannel(nullptr, 0);
    return;
}

SoundChannelHandle::SoundChannelHandle(Audio* audio, uint32_t channelId)
{
    SetSoundChannel(audio, channelId);
    return;
}

SoundChannelHandle::~SoundChannelHandle()
{
    try
    {
        Free();
    }
    catch(...)
    {
    }
}

SoundChannelHandle::SoundChannelHandle(const SoundChannelHandle& channel) noexcept
{
    channelId = channel.channelId;
    audio     = channel.audio;

    if (audio)
        audio->IncSoundChannelRefCount(channelId);

    return;
}

SoundChannelHandle::SoundChannelHandle(SoundChannelHandle&& channel) noexcept
{
    channelId = channel.channelId;
    audio     = channel.audio;

    channel.channelId = 0;
    channel.audio     = nullptr;

    return;
}

SoundChannelHandle& SoundChannelHandle::operator=(const SoundChannelHandle& channel) noexcept
{
    if (&channel == this)
        return *this;

    Free();

    channelId = channel.channelId;
    audio     = channel.audio;

    if (audio)
        audio->IncSoundChannelRefCount(channelId);

    return *this;
}

SoundChannelHandle& SoundChannelHandle::operator=(SoundChannelHandle&& channel) noexcept
{
    if (&channel == this)
        return *this;

    Free();

    channelId = channel.channelId;
    audio     = channel.audio;

    channel.channelId = 0;
    channel.audio     = nullptr;

    return *this;
}

bool SoundChannelHandle::operator==(const SoundChannelHandle& rhs) const noexcept
{
    if (!audio && !rhs.audio)
        return true;

    return audio==rhs.audio && channelId==rhs.channelId;
}

void SoundChannelHandle::SetSoundChannel(Audio* audio, uint32_t channelId)
{
    if(!audio)
    {
        this->channelId = 0;
        this->audio     = nullptr;
    }
    else
    {        
        this->audio     = audio;
        this->channelId = channelId;
        audio->IncSoundChannelRefCount(channelId);
    }

    return;
}

void SoundChannelHandle::Free()
{
    if (audio)
        audio->DecSoundChannelRefCount(channelId);

    channelId = 0;
    audio     = nullptr;
    return;
}

void SoundChannelHandle::Load(const SoundHandle& sound,
                              SoundEndRule soundEndRule,
                              double volume,
                              uint32_t sampleIndex)
{
    std::uint32_t newSound = 0;
    std::uint32_t oldSound = 0;

    SoundEndRule oldSoundEndRule;
    SoundEndRule newSoundEndRule;

    {
        lock_guard<mutex> lock(audio->audioMutex);

        auto it = audio->channels.find(channelId);
        if (it == audio->channels.end() || it->second == nullptr)
            throw error("Trying to load a sound into an invalid channel");

        if (audio->sounds.find(sound.soundId) == audio->sounds.end() && sound.soundId != 0)
            throw error("Trying to load an invalid sound into a channel");

        it->second->soundState   = SoundState::Stopped;
        it->second->volume       = volume;
        it->second->sampleIndex  = sampleIndex;

        oldSoundEndRule = it->second->soundEndRule; 
        newSoundEndRule = soundEndRule;
        it->second->soundEndRule = newSoundEndRule;
        
        oldSound = it->second->soundId;
        newSound = sound.soundId;
        it->second->soundId = newSound;
    }

    if (newSound != oldSound)
    {
        audio->IncSoundRefCount(newSound);
        audio->DecSoundRefCount(oldSound);
    }

    // If the sound end rule is StopAndDeleteChannel we want the audio object to take ownership.
    if (newSoundEndRule == SoundEndRule::StopAndDeleteChannel)
        audio->IncSoundChannelRefCount(channelId);

    if (oldSoundEndRule == SoundEndRule::StopAndDeleteChannel)
        audio->DecSoundChannelRefCount(channelId);

    return;
}

void SoundChannelHandle::LoadAndPlay(const SoundHandle& sound,
                                     SoundEndRule soundEndRule,
                                     double volume,
                                     uint32_t sampleIndex)
{
    Load(sound, soundEndRule, volume, sampleIndex);
    Play();
    return;
}

void SoundChannelHandle::Play()
{
    lock_guard<mutex> lock(audio->audioMutex);

    auto it = audio->channels.find(channelId);
    if (it == audio->channels.end() || it->second == nullptr)
        throw error("Trying to play audio on an invalid channel");

    std::uint32_t soundId = it->second->soundId;
    if (soundId==0 || audio->sounds.find(soundId)==audio->sounds.end())
        throw error("Trying to play an invalid sound");

    it->second->soundState = SoundState::Playing;

    return;
}

void SoundChannelHandle::Pause()
{
    lock_guard<mutex> lock(audio->audioMutex);

    auto it = audio->channels.find(channelId);
    if (it == audio->channels.end() || it->second == nullptr)
        throw error("Trying to pause audio on an invalid channel");

    it->second->soundState = SoundState::Stopped;

    return;
}

void SoundChannelHandle::Stop()
{
    lock_guard<mutex> lock(audio->audioMutex);

    auto it = audio->channels.find(channelId);
    if (it == audio->channels.end() || it->second == nullptr)
        throw error("Trying to stop audio on an invalid channel");

    it->second->soundState  = SoundState::Stopped;
    it->second->sampleIndex = 0;

    return;
}

void SoundChannelHandle::GetState(SoundState& state, uint32_t& sampleIndex)
{
    lock_guard<mutex> lock(audio->audioMutex);

    auto it = audio->channels.find(channelId);
    if (it == audio->channels.end() || it->second == nullptr)
        throw error("Trying to get state of audio on an invalid channel");

    state       = it->second->soundState;
    sampleIndex = it->second->sampleIndex;

    return;
}

bool SoundChannelHandle::IsPlaying()
{
    lock_guard<mutex> lock(audio->audioMutex);

    auto it = audio->channels.find(channelId);
    if (it == audio->channels.end() || it->second == nullptr)
        throw error("Trying to get state of audio on an invalid channel");

    return it->second->soundState == SoundState::Playing;
}

} // End of namespace mi.