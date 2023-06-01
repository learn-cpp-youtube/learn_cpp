#pragma once

#include <string>

namespace mi
{
namespace shader
{

// (0, 0) is the upper left corner of the upper left pixel,
// (1, 1) is the lower right corner of the lower right pixel.

namespace vertex
{
// blitToScreen flips the y position, so it renders correctly on the screen.
extern const std::string blitToScreen;
extern const std::string blitToImage;
extern const std::string drawToImage;
} // End of namespace mi::shader::vertex.

namespace fragment
{
extern const std::string blitToScreen;
extern const std::string createMask;
extern const std::string addAlpha;
extern const std::string drawToImage;
} // End of namespace mi::shader::fragment.

} // End of namespace mi::shader.

} // End of namespace mi.

