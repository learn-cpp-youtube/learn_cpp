#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace mi
{

//Currently only handles 24bit and 32bit Bitmaps (alpha values are ignored).
//Top Left pixel = (0,0).
class BitmapHelper
{
public:
    void SetDimensions(long width, long height);

    //The bitmap should be uncompressed with a bitCount of 24 or 32 (negative heights are supported).
    void Load(const std::string& filename);

    //The file will be overwritten if it exists.
    void Save(const std::string& filename) const;

    std::vector<std::uint8_t> data;
    std::uint32_t width;
    std::uint32_t height;
};

} //End of namespace mi.