#include "WavHelper.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdint>

using error = std::runtime_error;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::string;
using std::uint32_t;

namespace
{

uint32_t ReadUnsignedInt(istream& in, long bytes)
{
    uint32_t value = 0;
    for (long i=0; i<bytes; i++)
        value |= uint8_t(in.get()) << (8*i);
    return value;
}

void WriteUnsignedInt(ostream& out, long bytes, uint32_t value)
{
    for (long i=0; i<bytes; i++)
        out.put((value>>(8*i)) & 0xff);
    return;
}

} // End of namespace.

namespace mi
{

void WavHelper::Load(const string& filename)
{
    ifstream       file;
    unsigned short numChannels;
    unsigned long  dataSize;

    file.open(filename, std::ios::binary);
    if (file.good() == false)
        throw error("WavHelper.Load() : Could not open file.");

    if (ReadUnsignedInt(file,4) != 0x46464952UL) // "RIFF" in ASCII
        throw error("WavHelper.Load() : Expected to read chunk id 'RIFF'.");
    
    ReadUnsignedInt(file, 4); // File size - 8 (For "RIFF", and the Unsigned Int just read).

    if (ReadUnsignedInt(file,4) != 0x45564157UL) // "WAVE" in ASCII
        throw error("WavHelper.Load() : Format is not 'WAVE'.");

    if (ReadUnsignedInt(file,4) != 0x20746d66UL) // "fmt " in ASCII
        throw error("WavHelper.Load() : Subchunk1 ID is not 'fmt '.");

    if (ReadUnsignedInt(file,4) != 16) // Format description is 16 bytes for PCM.
        throw error("WavHelper.Load() : Unexpected format size for a PCM Wav file.");

    if (ReadUnsignedInt(file,2) != 1) // Audio format = 1 (PCM).
        throw error("WavHelper.Load() : Audio format is not PCM.");

    numChannels = ReadUnsignedInt(file,2);
    
    if (numChannels == 0)
        throw error("WavHelper.Load() : The number of channels is 0.");

    sampleRate = ReadUnsignedInt(file,4);
    
    ReadUnsignedInt(file, 4); // Byte rate.
    ReadUnsignedInt(file, 2); // Block align.

    if (ReadUnsignedInt(file,2) != 16) // Bits per sample.
        throw error("WavHelper.Load() : Only 16 bit samples are currently supported.");

    if (ReadUnsignedInt(file,4) != 0x61746164UL) // "data" in ASCII
        throw error("WavHelper.Load() : Subchunk2 ID is not 'data'.");

    dataSize = ReadUnsignedInt(file,4);

    // Read in channels.
    amplitudes.resize(numChannels);
    for (long i=0; i<numChannels; i++)
        amplitudes[i].resize(0);

    for (size_t i=0; i<dataSize/2/numChannels; i++)
    for (long j=0; j<numChannels; j++)
        amplitudes[j].push_back(ReadUnsignedInt(file,2) | 0xffff0000);

    file.close();
    return;
}

void WavHelper::Save(const string& filename) const
{
    ofstream file;

    // Check for errors.
    unsigned long numChannels = static_cast<unsigned long>(amplitudes.size());
    unsigned long numSamples  = 0;

    if (numChannels > 0)
        numSamples = static_cast<unsigned long>(amplitudes[0].size());

    if (numChannels == 0)
        throw error("WavHelper.Save() : There must be at least one channel.");

    for (size_t i=0; i<numChannels; i++)
        if (amplitudes[i].size() != numSamples)
            throw error("WavHelper.Save() : The number of samples in each channel must be the same.");

    // Open the file.
    file.open(filename,std::ios::binary);

    if(file.good() == false)
        throw error("WavHelper.Save() : Could not open file.");

    // Set the wav file header.
    unsigned long dataSize = numSamples*numChannels*2; // NumSamples*NumChannels*BitsPerSample/8.

    WriteUnsignedInt(file, 4, 0x46464952UL);             // "RIFF" in ASCII
    WriteUnsignedInt(file, 4, dataSize+36);              // FileSize-8. (FileSize = DataSize+44)
    WriteUnsignedInt(file, 4, 0x45564157UL);             // "WAVE" in ASCII
    WriteUnsignedInt(file, 4, 0x20746d66UL);             // "fmt " in ASCII
    WriteUnsignedInt(file, 4, 16);                       // Format description takes 16 bytes for PCM.
    WriteUnsignedInt(file, 2, 1);                        // Audio Format is PCM (=1).
    WriteUnsignedInt(file, 2, numChannels);              // NumChannels.
    WriteUnsignedInt(file, 4, sampleRate);               // SampleRate.
    WriteUnsignedInt(file, 4, sampleRate*numChannels*2); // ByteRate   = SampleRate*NumChannels*BitsPerSample/8.
    WriteUnsignedInt(file, 2, numChannels*2);            // BlockAlign = NumChannels*BitsPerSample/8.
    WriteUnsignedInt(file, 2, 16);                       // BitsPerSample.
    WriteUnsignedInt(file, 4, 0x61746164UL);             // "data" in ASCII
    WriteUnsignedInt(file, 4, dataSize);                 // DataSize.

    // Write the amplitude data.
    for (size_t j=0; j<amplitudes[0].size(); j++)
    for (size_t i=0; i<amplitudes.size(); i++)
        WriteUnsignedInt(file, 2, amplitudes[i][j]);

    file.close();
    return;
}

} // End of namespace mi.