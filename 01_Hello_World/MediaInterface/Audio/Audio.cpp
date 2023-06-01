#include "Audio.h"
#include "SoundChannelHandle.h"
#include "SoundHandle.h"
#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>
#include <iostream>
#include <utility>

using error = std::runtime_error;
using std::string;
using std::uint8_t;
using std::uint32_t;
using std::vector;
using std::deque;
using std::mutex;
using std::lock_guard;
using std::cout;
using std::endl;

// For debug purposes.
/*
static void DisplayAudioSpec(string title, const SDL_AudioSpec& spec)
{
    cout << title << endl;
    cout << "Format   : ";
    cout << (SDL_AUDIO_MASK_BITSIZE&spec.format) << " Bit ";
    cout << ((SDL_AUDIO_MASK_SIGNED&spec.format)?"Signed":"Unsigned") << " ";
    cout << ((SDL_AUDIO_MASK_ENDIAN&spec.format)?"Big":"Little") << " Endian ";
    cout << ((SDL_AUDIO_MASK_DATATYPE&spec.format)?"Float":"Integer") << endl;    
    cout << "Freq     : " << spec.freq << endl;
    cout << "Channels : " << long(spec.channels) << endl;
    cout << "Samples  : " << spec.samples << endl;
    cout << "User     : " << spec.userdata << endl;
    cout << "Callback : " << spec.callback << endl;
    cout << "Padding  : " << spec.padding << endl;
    cout << "Silence  : " << long(spec.silence) << endl;
    cout << "Size     : " << spec.size << endl;
    cout << endl;

    return;
}
*/

namespace mi
{

Audio::Audio()
{
    initialized    = false;
    callbackIsGood = false;
    return;
}

Audio::~Audio()
{
    Free();
    return;
}

void Audio::CallbackWrapper(void* userData, uint8_t* stream, int length)
{
    static_cast<Audio*>(userData)->Callback(stream, length);
    return;
}

void Audio::Callback(uint8_t* stream, uint32_t length)
{
    lock_guard<mutex> lock(audioMutex);

    if (!callbackIsGood)
    {
        // Fill with silence.
        while (length > 0)
            ConvertDouble(0, stream, length);
        return;
    }

    try
    {
        vector<uint32_t> playing;
        for (auto& it : channels)
        {
            if (it.second && it.second->soundState==SoundState::Playing && it.second->soundId!=0)
                playing.push_back(it.first);
        }

        uint8_t* buf = stream;
        vector<uint32_t> deleteList;
        Channel* pC;

        while (length > 0)
        {
            for (long c=0; c<numChannels; c++)
            {
                if (c >= 2)
                {
                    ConvertDouble(0, buf, length);
                    continue;
                }

                double result = 0;

                for (uint32_t channelId : playing)
                {
                    pC = channels[channelId].get();

                    if (pC->soundState == SoundState::Stopped)
                        continue;

                    Sound* pS = sounds[pC->soundId].get();

                    if (pC->sampleIndex >= pS->left.size())
                    {
                        // Reached the end of the sound.
                        if (pC->soundEndRule == SoundEndRule::Loop)
                            pC->sampleIndex = 0;
                        else
                        {
                            if (pC->soundEndRule == SoundEndRule::StopAndDeleteChannel)
                                deleteList.push_back(channelId);

                            pC->soundState = SoundState::Stopped;
                            continue;
                        }
                    }

                    if (c == 0)
                        result += pC->volume * pS->left[pC->sampleIndex];

                    if (c == 1 || numChannels == 1)
                    {
                        result += pC->volume * pS->right[pC->sampleIndex];
                        pC->sampleIndex++;
                    }
                }

                if (numChannels == 1)
                    result /= 2;

                if (result < -1)
                    result = -1;
                if (result > 1)
                    result = 1;

                ConvertDouble(result, buf, length);
            }
        }

        // Cleanup any channels that should be deleted.
        for (uint32_t channelId : deleteList)
        {
            // Delete the associated sound if appropriate.
            std::uint32_t soundId = channels[channelId]->soundId;
            if (soundId!=0 && sounds.find(soundId)!=sounds.end())
            {
                std::uint32_t& refCount = sounds[soundId]->refCount;
                refCount--;

                if (refCount == 0)
                {
                    sounds.erase(soundId);
                    unusedSoundIds.push_front(soundId);
                }
            }

            channels[channelId]->soundId = 0;

            // Delete the channel if appropriate.
            std::uint32_t& refCount = channels[channelId]->refCount;
            refCount--;

            if (refCount == 0)
            {
                channels.erase(channelId);
                unusedChannelIds.push_front(channelId);
            }
        }
    }
    catch(...)
    {
        // We can't let exceptions escape, as this function is being called in a thread.
        callbackIsGood = false;
    }

    return;
}

void Audio::Init()
{
    Free();

    uint32_t test = 1;
    systemIsBigEndian = reinterpret_cast<uint8_t*>(&test)[3];

    bool isSigned;
    bool isBigEndian;
    bool isFloat;
    long numBytes;

    unusedChannelIds.push_back(0);
    unusedSoundIds.push_back(1); // soundId 0 represents no sound set.

    SDL_AudioSpec specTarget;

    specTarget.format   = AUDIO_S16LSB;
    specTarget.freq     = 44100;
    specTarget.channels = 2;
    specTarget.samples  = 1024;
    specTarget.userdata = static_cast<void*>(this);
    specTarget.callback = Audio::CallbackWrapper;
    specTarget.padding  = 0;
    specTarget.silence  = 0;
    specTarget.size     = 0;

    //DisplayAudioSpec("Target", specTarget);

    device = SDL_OpenAudioDevice(nullptr, 0, &specTarget, &spec, SDL_AUDIO_ALLOW_ANY_CHANGE);

    //DisplayAudioSpec("Spec", spec);

    if (device <= 0)
        throw error(string("Opening SDL audio device failed: ")+SDL_GetError());

    isSigned    = SDL_AUDIO_MASK_SIGNED   & spec.format;
    isBigEndian = SDL_AUDIO_MASK_ENDIAN   & spec.format;
    isFloat     = SDL_AUDIO_MASK_DATATYPE & spec.format;
    numBytes    = (SDL_AUDIO_MASK_BITSIZE & spec.format)/8;

    // Explicitly store the number of channels, and the sample rate to make it easier to abstract
    // away the SDL layer.
    numChannels = spec.channels;
    sampleRate  = spec.freq;

    if (isBigEndian == systemIsBigEndian)
    {
        if (isFloat)
            ConvertDouble = DoubleToFS;
        else
        {
            if (isSigned)
            {
                if (numBytes == 1)
                    ConvertDouble = DoubleTo8SI;
                else
                if (numBytes == 2)
                    ConvertDouble = DoubleTo16SIS;
                else
                if (numBytes == 4)
                    ConvertDouble = DoubleTo32SIS;
            }
            else
            {
                if (numBytes == 1)
                    ConvertDouble = DoubleTo8UI;
                else
                if (numBytes == 2)
                    ConvertDouble = DoubleTo16UIS;
                else
                if (numBytes == 4)
                    ConvertDouble = DoubleTo32UIS;
            }
        }
    }
    else
    {
        if (isFloat)
            ConvertDouble = DoubleToFN;
        else
        {
            if (isSigned)
            {
                if (numBytes == 1)
                    ConvertDouble = DoubleTo8SI;
                else
                if (numBytes == 2)
                    ConvertDouble = DoubleTo16SIN;
                else
                if (numBytes == 4)
                    ConvertDouble = DoubleTo32SIN;
            }
            else
            {
                if (numBytes == 1)
                    ConvertDouble = DoubleTo8UI;
                else
                if (numBytes == 2)
                    ConvertDouble = DoubleTo16UIN;
                else
                if (numBytes == 4)
                    ConvertDouble = DoubleTo32UIN;
            }
        }
    }

    callbackIsGood = true;

    SDL_PauseAudioDevice(device, 0);

    initialized = true;
    return;
}

void Audio::Free()
{
    // Just in case, get the mutex.
    lock_guard<mutex> lock(audioMutex);

    // Stop all audio.
    if (initialized)
    {
        SDL_PauseAudioDevice(device, 1);
        SDL_CloseAudioDevice(device);
    }

    callbackIsGood = false;

    // Clean the maps.
    channels.clear();
    unusedChannelIds.clear();

    sounds.clear();
    unusedSoundIds.clear();

    initialized = false;
    return;
}

SoundChannelHandle Audio::CreateSoundChannel()
{
    uint32_t id;
    std::unique_ptr<Channel> pChannel = std::make_unique<Channel>();

    pChannel->soundState   = SoundState::Stopped;
    pChannel->soundEndRule = SoundEndRule::Stop;
    pChannel->volume       = 0;
    pChannel->sampleIndex  = 0;
    pChannel->soundId      = 0;

    {
        lock_guard<mutex> lock(audioMutex);

        id = unusedChannelIds.front();

        if (unusedChannelIds.size() == 1)
            unusedChannelIds.front()++;
        else
            unusedChannelIds.pop_front();

        channels[id] = std::move(pChannel);
    }
    
    return SoundChannelHandle(this, id);
}

SoundHandle Audio::CreateSound(const string& filename)
{
    WavHelper wavHelper;
    wavHelper.Load(filename);
    return CreateSound(wavHelper);
}

SoundHandle Audio::CreateSound(const WavHelper& wavHelper)
{
    uint32_t id = 0;
    std::unique_ptr<Sound> pSound = std::make_unique<Sound>();

    Convert(sampleRate, wavHelper, pSound.get());

    if (pSound->left.size() == 0)
        throw error("Sound contains zero samples");

    {
        lock_guard<mutex> lock(audioMutex);

        id = unusedSoundIds.front();

        if (unusedSoundIds.size() == 1)
            unusedSoundIds.front()++;
        else
            unusedSoundIds.pop_front();
        
        sounds[id] = std::move(pSound);
    }

    return SoundHandle(this, id);
}

void Audio::IncSoundChannelRefCount(std::uint32_t channelId)
{
    lock_guard<mutex> lock(audioMutex);

    auto it = channels.find(channelId);

    if (it == channels.end())
        throw error("Couldn't find sound channel reference count");

    it->second->refCount++;
    return;
}

void Audio::DecSoundChannelRefCount(std::uint32_t channelId)
{
    lock_guard<mutex> lock(audioMutex);

    auto it = channels.find(channelId);
    if (it == channels.end())
        return; // Already deleted.

    it->second->refCount--;
    if (it->second->refCount)
        return;

    // Delete the associated sound if appropriate.
    std::uint32_t soundId = it->second->soundId;
    if (soundId!=0 && sounds.find(soundId)!=sounds.end())
    {
        std::uint32_t& refCount = sounds[soundId]->refCount;
        refCount--;

        if (refCount == 0)
        {
            sounds.erase(soundId);
            unusedSoundIds.push_front(soundId);
        }
    }

    channels.erase(channelId);
    unusedChannelIds.push_front(channelId);

    return;
}

void Audio::IncSoundRefCount(std::uint32_t soundId)
{
    if (soundId == 0)
        return;

    lock_guard<mutex> lock(audioMutex);

    auto it = sounds.find(soundId);

    if (it == sounds.end())
        throw error("Couldn't find sound reference count");

    it->second->refCount++;
    return;
}

void Audio::DecSoundRefCount(std::uint32_t soundId)
{
    if (soundId == 0)
        return;

    lock_guard<mutex> lock(audioMutex);
       
    auto it = sounds.find(soundId);

    if (it == sounds.end())
        return; // Already deleted.

    Sound* pSound = it->second.get();
    pSound->refCount--;

    if (pSound->refCount)
        return;

    vector<uint32_t> deleteList;

    // Check which channels are playing the sound and stop them.
    for (auto& it : channels)
    {
        if (it.second && it.second->soundId == soundId)
        {
            // Check if we should delete the channel.
            if (it.second->soundEndRule == SoundEndRule::StopAndDeleteChannel)
                deleteList.push_back(it.first);

            it.second->soundState = SoundState::Stopped;
            it.second->soundId    = 0;
        }
    }

    sounds.erase(soundId);
    unusedSoundIds.push_front(soundId);

    // Cleanup any channels that should be deleted.
    for (uint32_t channelId : deleteList)
    {
        std::uint32_t& refCount = channels[channelId]->refCount;
        refCount--;

        if (refCount == 0)
        {
            channels.erase(channelId);
            unusedChannelIds.push_front(channelId);
        }
    }

    return;
}

void Audio::Convert(unsigned long targetSampleRate, const WavHelper& wavHelper, Sound* pSound)
{
    // Only consider the first two channels of the input.
    // If there is only one channel duplicate it to get two channels.

    vector<double> leftChannel;
    vector<double> rightChannel;

    // Convert input data to doubles.
    long numChannels = static_cast<long>(wavHelper.amplitudes.size());
    unsigned long numSamples  = 0;
    
    if (numChannels > 0)
        numSamples = static_cast<unsigned long>(wavHelper.amplitudes[0].size());

    if (numSamples <= 1)
        throw "Audio.Convert() : Not enough samples";

    // Fill in the left channel.
    for (size_t n=0; n<numSamples; n++)
        leftChannel.push_back(wavHelper.amplitudes[0][n]/32768.0);

    // Fill in the right channel.
    if (numChannels <= 1)
        rightChannel = leftChannel;
    else
    {
        for (size_t n=0; n<numSamples; n++)
            rightChannel.push_back(wavHelper.amplitudes[1][n]/32768.0);
    }

    // Interpolate frequency.
    vector<float>  temp;
    vector<double> channel;
    double totalSamples = (leftChannel.size()-1)
                          *static_cast<double>(targetSampleRate)/wavHelper.sampleRate
                          +1;

    for (long c=0; c<2; c++)
    {
        if (c==0)
            channel.swap(leftChannel);
        else
            channel.swap(rightChannel);

        temp.clear();
        temp.push_back(static_cast<float>(channel[0]));
        while (temp.size() < totalSamples)
        {
            double index  = (temp.size())*static_cast<double>(wavHelper.sampleRate)/targetSampleRate;
            long   lower  = static_cast<long>(index);
            long   upper  = lower+1;
            double factor = index-lower;

            if (lower <= 0)
                lower = 0;

            if (upper >= static_cast<long>(channel.size()))
                upper = static_cast<long>(channel.size()-1);

            temp.push_back(static_cast<float>(channel[lower]*(1-factor)+channel[upper]*factor));
        }

        if (c==0)
            temp.swap(pSound->left);
        else
            temp.swap(pSound->right);
    }

    return;
}

void Audio::DoubleTo8UI(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::uint8_t z   = static_cast<std::uint8_t>((input+1)/2 * 255);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[0];
    buffer += 1;
    length -= 1;
    return;
}

void Audio::DoubleTo8SI(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::int8_t z   = static_cast<std::int8_t>(input*127);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[0];
    buffer += 1;
    length -= 1;
    return;
}

void Audio::DoubleTo16UIS(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::uint16_t z  = static_cast<std::uint16_t>((input+1)/2 * 65535);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[0];
    buffer[1] = pBytes[1];
    buffer += 2;
    length -= 2;
    return;
}

void Audio::DoubleTo16UIN(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::uint16_t z  = static_cast<std::uint16_t>((input+1)/2 * 65535);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[1];
    buffer[1] = pBytes[0];
    buffer += 2;
    length -= 2;
    return;
}

void Audio::DoubleTo16SIS(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::int16_t z  = static_cast<std::int16_t>(input*32767);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[0];
    buffer[1] = pBytes[1];
    buffer += 2;
    length -= 2;
    return;
}

void Audio::DoubleTo16SIN(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::int16_t z  = static_cast<std::int16_t>(input*32767);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[1];
    buffer[1] = pBytes[0];
    buffer += 2;
    length -= 2;
    return;
}

void Audio::DoubleTo32UIS(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::uint32_t z = static_cast<std::uint32_t>((input+1)/2 * 4294967295);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[0];
    buffer[1] = pBytes[1];
    buffer[2] = pBytes[2];
    buffer[3] = pBytes[3];
    buffer += 4;
    length -= 4;
    return;
}

void Audio::DoubleTo32UIN(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::uint32_t z = static_cast<std::uint32_t>((input+1)/2 * 4294967295);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[3];
    buffer[1] = pBytes[2];
    buffer[2] = pBytes[1];
    buffer[3] = pBytes[0];
    buffer += 4;
    length -= 4;
    return;
}

void Audio::DoubleTo32SIS(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::int32_t z  = static_cast<std::int32_t>(input*2147483647);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[0];
    buffer[1] = pBytes[1];
    buffer[2] = pBytes[2];
    buffer[3] = pBytes[3];
    buffer += 4;
    length -= 4;
    return;
}

void Audio::DoubleTo32SIN(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    std::int32_t z  = static_cast<std::int32_t>(input*2147483647);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&z);
    buffer[0] = pBytes[3];
    buffer[1] = pBytes[2];
    buffer[2] = pBytes[1];
    buffer[3] = pBytes[0];
    buffer += 4;
    length -= 4;
    return;
}

void Audio::DoubleToFS(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    float f = static_cast<float>(input);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&f);
    buffer[0] = pBytes[0];
    buffer[1] = pBytes[1];
    buffer[2] = pBytes[2];
    buffer[3] = pBytes[3];
    buffer += 4;
    length -= 4;
    return;
}

void Audio::DoubleToFN(double input, std::uint8_t*& buffer, std::uint32_t& length)
{
    float f = static_cast<float>(input);
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(&f);
    buffer[0] = pBytes[3];
    buffer[1] = pBytes[2];
    buffer[2] = pBytes[1];
    buffer[3] = pBytes[0];
    buffer += 4;
    length -= 4;
    return;
}

} // End of namespace mi.
