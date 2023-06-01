#pragma once

#include <cstdint>
#include <cstdlib>
#include <concepts>

namespace mi
{

class Color
{
public:
    constexpr Color() noexcept : r{0}, g{0}, b{0}, a{0} {}

    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a=255) noexcept
        : r{r}, g{g}, b{b}, a{a} {}

    constexpr Color(std::uint32_t c) noexcept
        : r((c>>24)&0xff), g((c>>16)&0xff), b((c>>8)&0xff), a(c&0xff) {}

    constexpr operator std::uint32_t() const noexcept
    {
        return ((std::uint32_t)r)<<24 | g<<16 | b<<8 | a;
    }

    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
    
    // If hue is a floating type it must be in [0, 1) Red = 0, Green = 1/3, Blue = 2/3.
    // If hue is an integral type it must be in [0, 1530) Red = 0, Green = 510, Blue = 1020.
    template<typename T> requires std::integral<T> || std::floating_point<T>
    static constexpr Color Hue(T hue) noexcept
    {
        std::int32_t input;

        if constexpr(std::floating_point<T>)
            input = static_cast<std::int32_t>(static_cast<double>(hue)*1530+0.5);
        else
            input = static_cast<int32_t>(hue);

        if (input < 0)
            input = 0;

        if (input >= 1530)
            input = 1529;

        std::uint32_t offset[3] = {0, 1020, 510};
        std::uint32_t c[3] = {};
        std::uint32_t h;

        for (std::size_t i=0; i<3; i++)
        {
            h = (input + offset[i]) % 1530;

            if (0 <= h && h < 255)
                c[i] = 255;
            else
            if (255 <= h && h < 510)
                c[i] = 510 - h;
            else
            if (510 <= h && h < 1020)
                c[i] = 0;
            else
            if (1020 <= h && h < 1275)
                c[i] = h - 1020;
            else
            if (1275 <= h && h < 1530)
                c[i] = 255;
        }

        return Color(static_cast<std::uint8_t>(c[0]),
                     static_cast<std::uint8_t>(c[1]),
                     static_cast<std::uint8_t>(c[2]));
    }
};

} // End of namespace mi.


