#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace mi
{

// Currently only handles uncompressed 16 bit linear PCM.
class WavHelper
{
public:
    // Only uncompressed 16 bit linear PCM wav files are supported.
    void Load(const std::string& filename);

    // The file will be overwritten if it exists.
    void Save(const std::string& filename) const;

    unsigned long sampleRate;
    std::vector<std::vector<std::int16_t>> amplitudes; // [channel][sample].
};

} // End of namespace mi.