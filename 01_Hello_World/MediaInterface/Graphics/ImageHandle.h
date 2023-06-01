#pragma once

#include "Graphics.h"
#include "Color.h"
#include <cstdint>
#include <vector>
#include <concepts>

// NOTE: There's a compiler bug where const templated member functions which have auto parameters
// have their constness ignored. As a result a temporary workaround of using 
// const ImageHandle& temp = *this;
// is being employed to ensure the overload resolution doesn't cause infinite recursion.

namespace mi
{

class ImageHandle
{
public:
    ImageHandle() noexcept;
    ~ImageHandle();

    ImageHandle(const ImageHandle& img) noexcept;
    ImageHandle(ImageHandle&& img) noexcept;

    ImageHandle& operator=(const ImageHandle& img) noexcept;
    ImageHandle& operator=(ImageHandle&& img) noexcept;

    bool operator==(const ImageHandle& rhs) const noexcept;
    bool operator!=(const ImageHandle& rhs) const noexcept { return !(*this == rhs); }
    
    std::uint32_t GetWidth()      const noexcept { return width;  }
    std::uint32_t GetHeight()     const noexcept { return height; }
    std::uint32_t GetSmoothFlag() const noexcept { return smooth; }

    std::vector<std::uint8_t> GetPixelData() const;

    void Clear(const Color& c) const;

    // Same as calling DrawScaledImage with destW, destH, maskW, maskH given by srcW, srcH.  
    void DrawImage(float destX, float destY,
                   const ImageHandle& srcImg,
                   float srcX,  float srcY,  float srcW,  float srcH,
                   float alpha = 255,
                   const ImageHandle* pMaskImg = nullptr,
                   float maskX = 0, float maskY = 0,
                   const ImageHandle* pPaletteImg = nullptr,
                   float paletteX = 0, float paletteY = 0, float paletteW = 0) const
    {
        DrawScaledImage(destX, destY, srcW, srcH,
                        srcImg,
                        srcX, srcY, srcW, srcH,
                        alpha,
                        pMaskImg,
                        maskX, maskY, srcW, srcH,
                        pPaletteImg,
                        paletteX, paletteY, paletteW);
    }
    void DrawScaledImage(float destX, float destY, float destW, float destH,
                         const ImageHandle& srcImg,
                         float srcX,  float srcY,  float srcW,  float srcH,
                         float alpha = 255,
                         const ImageHandle* pMaskImg = nullptr,
                         float maskX = 0, float maskY = 0, float maskW = 0, float maskH = 0,
                         const ImageHandle* pPaletteImg = nullptr,
                         float paletteX = 0, float paletteY = 0, float paletteW = 0) const;
    void DrawTriangle(float x1, float y1, const Color& c1,
                      float x2, float y2, const Color& c2,
                      float x3, float y3, const Color& c3) const;
    void DrawTriangle(float x1, float y1,
                      float x2, float y2,
                      float x3, float y3,
                      const Color& c) const;
    void DrawRectangle(float x, float y, float width, float height, const Color& c) const;
    void DrawCircle(float cx, float cy, float radius, const Color& c) const;
    void DrawLine(float x1, float y1, float x2, float y2, float thickness, const Color& c) const;



    // Helper functions to avoid conversion warnings.
    template<std::convertible_to<float> DX,         std::convertible_to<float> DY,
             std::convertible_to<float> SX,         std::convertible_to<float> SY,
             std::convertible_to<float> SW,         std::convertible_to<float> SH,
             std::convertible_to<float> A  = float,
             std::convertible_to<float> MX = float, std::convertible_to<float> MY = float,
             std::convertible_to<float> PX = float, std::convertible_to<float> PY = float,
             std::convertible_to<float> PW = float>
    void DrawImage(DX destX, DY destY,
                   const ImageHandle& srcImg,
                   SX srcX,  SY srcY,  SW srcW,  SH srcH,
                   A alpha = 255,
                   const ImageHandle* pMaskImg = nullptr,
                   MX maskX = 0, MY maskY = 0,
                   const ImageHandle* pPaletteImg = nullptr,
                   PX paletteX = 0, PY paletteY = 0, PW paletteW = 0) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawImage(static_cast<float>(destX),    static_cast<float>(destY),
                  srcImg,
                  static_cast<float>(srcX),     static_cast<float>(srcY),
                  static_cast<float>(srcW),     static_cast<float>(srcH),
                  static_cast<float>(alpha),
                  pMaskImg,
                  static_cast<float>(maskX),    static_cast<float>(maskY),
                  pPaletteImg,
                  static_cast<float>(paletteX), static_cast<float>(paletteY),
                  static_cast<float>(paletteW));
    }

    template<std::convertible_to<float> DX,         std::convertible_to<float> DY,
             std::convertible_to<float> DW,         std::convertible_to<float> DH,
             std::convertible_to<float> SX,         std::convertible_to<float> SY,
             std::convertible_to<float> SW,         std::convertible_to<float> SH,
             std::convertible_to<float> A  = float,
             std::convertible_to<float> MX = float, std::convertible_to<float> MY = float,
             std::convertible_to<float> MW = float, std::convertible_to<float> MH = float,
             std::convertible_to<float> PX = float, std::convertible_to<float> PY = float,
             std::convertible_to<float> PW = float>
    void DrawScaledImage(DX destX, DY destY, DW destW, DH destH,
                         const ImageHandle& srcImg,
                         SX srcX,  SY srcY,  SW srcW,  SH srcH,
                         A alpha = 255,
                         const ImageHandle* pMaskImg = nullptr,
                         MX maskX = 0, MY maskY = 0, MW maskW = 0, MH maskH = 0,
                         const ImageHandle* pPaletteImg = nullptr,
                         PX paletteX = 0, PY paletteY = 0, PW paletteW = 0) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawScaledImage(static_cast<float>(destX),    static_cast<float>(destY),
                        static_cast<float>(destW),    static_cast<float>(destH),
                        srcImg,
                        static_cast<float>(srcX),     static_cast<float>(srcY),
                        static_cast<float>(srcW),     static_cast<float>(srcH),
                        static_cast<float>(alpha),
                        pMaskImg,
                        static_cast<float>(maskX),    static_cast<float>(maskY),
                        static_cast<float>(maskW),    static_cast<float>(maskH),
                        pPaletteImg,
                        static_cast<float>(paletteX), static_cast<float>(paletteY),
                        static_cast<float>(paletteW));
    }

    void DrawTriangle(std::convertible_to<float> auto x1, std::convertible_to<float> auto y1,
                      const Color& c1,
                      std::convertible_to<float> auto x2, std::convertible_to<float> auto y2, 
                      const Color& c2,
                      std::convertible_to<float> auto x3, std::convertible_to<float> auto y3,
                      const Color& c3) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawTriangle(static_cast<float>(x1), static_cast<float>(y1), c1,
                     static_cast<float>(x2), static_cast<float>(y2), c2,
                     static_cast<float>(x3), static_cast<float>(y3), c3);
    }

    void DrawTriangle(std::convertible_to<float> auto x1, std::convertible_to<float> auto y1,
                      std::convertible_to<float> auto x2, std::convertible_to<float> auto y2,
                      std::convertible_to<float> auto x3, std::convertible_to<float> auto y3,
                      const Color& c) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawTriangle(static_cast<float>(x1), static_cast<float>(y1),
                     static_cast<float>(x2), static_cast<float>(y2),
                     static_cast<float>(x3), static_cast<float>(y3),
                     c);
    }

    void DrawRectangle(std::convertible_to<float> auto x,
                       std::convertible_to<float> auto y,
                       std::convertible_to<float> auto width,
                       std::convertible_to<float> auto height,
                       const Color& c) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawRectangle(static_cast<float>(x), static_cast<float>(y),
                      static_cast<float>(width), static_cast<float>(height), c);
    }

    void DrawCircle(std::convertible_to<float> auto cx, std::convertible_to<float> auto cy,
                    std::convertible_to<float> auto radius, const Color& c) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawCircle(static_cast<float>(cx), static_cast<float>(cy), static_cast<float>(radius), c);
    }

    void DrawLine(std::convertible_to<float> auto x1, std::convertible_to<float> auto y1,
                  std::convertible_to<float> auto x2, std::convertible_to<float> auto y2,
                  std::convertible_to<float> auto thickness, const Color& c) const
    {
        const ImageHandle& temp = *this; // Workaround for compiler bug.
        temp.                            // Workaround for compiler bug.
        DrawLine(static_cast<float>(x1), static_cast<float>(y1),
                 static_cast<float>(x2), static_cast<float>(y2),
                 static_cast<float>(thickness), c);
    }

private:
    ImageHandle(Graphics* graphics, std::uint32_t texture) noexcept;

    void SetTexture(Graphics* graphics, std::uint32_t texture) noexcept;
    void Free();
    
    std::uint32_t texture  = 0;
    Graphics*     graphics = nullptr;
    std::uint32_t width;
    std::uint32_t height;
    bool          smooth;

    friend class MediaInterface;
};

} // End of namespace mi.