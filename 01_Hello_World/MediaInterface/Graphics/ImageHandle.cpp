#include "ImageHandle.h"
#include "Graphics.h"
#include <cstdint>
#include <cmath>

using std::uint32_t;
using std::sin;
using std::cos;
using std::sqrt;

const double PI = 3.1415926535897932384626433832795;

namespace mi
{

ImageHandle::ImageHandle() noexcept
{
    SetTexture(nullptr, 0);
    return;
}

ImageHandle::ImageHandle(Graphics* graphics, uint32_t texture) noexcept
{
    SetTexture(graphics, texture);
    return;
}

ImageHandle::~ImageHandle()
{
    try
    {
        Free();
    }
    catch(...)
    {
    }
}

ImageHandle::ImageHandle(const ImageHandle& img) noexcept
{
    texture  = img.texture;
    graphics = img.graphics;
    width    = img.width;
    height   = img.height;
    smooth   = img.smooth;

    if (graphics)
        graphics->GetTextureRefCount(texture)++;

    return;
}

ImageHandle::ImageHandle(ImageHandle&& img) noexcept
{
    texture  = img.texture;
    graphics = img.graphics;
    width    = img.width;
    height   = img.height;
    smooth   = img.smooth;

    img.texture  = 0;
    img.graphics = nullptr;

    return;
}

ImageHandle& ImageHandle::operator=(const ImageHandle& img) noexcept
{
    if (&img == this)
        return *this;

    Free();

    texture  = img.texture;
    graphics = img.graphics;
    width    = img.width;
    height   = img.height;
    smooth   = img.smooth;

    if (graphics)
        graphics->GetTextureRefCount(texture)++;

    return *this;
}

ImageHandle& ImageHandle::operator=(ImageHandle&& img) noexcept
{
    if (&img == this)
        return *this;

    Free();

    texture  = img.texture;
    graphics = img.graphics;
    width    = img.width;
    height   = img.height;
    smooth   = img.smooth;

    img.texture  = 0;
    img.graphics = nullptr;

    return *this;
}

bool ImageHandle::operator==(const ImageHandle& rhs) const noexcept
{
    return texture==rhs.texture && graphics==rhs.graphics;
}

void ImageHandle::SetTexture(Graphics* graphics, uint32_t texture) noexcept
{
    Free();

    if(!graphics || !texture)
    {
        this->texture  = 0;
        this->graphics = nullptr;
        width  = 0;
        height = 0;
        smooth = false;
    }
    else
    {
        this->texture  = texture;
        this->graphics = graphics;
        width  = graphics->GetTextureWidth(texture);
        height = graphics->GetTextureHeight(texture);
        smooth = graphics->GetTextureSmoothFlag(texture);
        graphics->GetTextureRefCount(texture)++;
    }

    return;
}

void ImageHandle::Free()
{
    if (!graphics)
        return;

    auto& refCount = graphics->GetTextureRefCount(texture);
    if (refCount == 1)
        graphics->DeleteTexture(texture);
    else
        refCount--;

    texture  = 0;
    graphics = nullptr;
    return;
}

void ImageHandle::Clear(const Color& c) const
{
    graphics->ClearTexture(texture, c);
    return;
}

std::vector<std::uint8_t> ImageHandle::GetPixelData() const
{
    std::vector<std::uint8_t> pixels;
    graphics->GetTexturePixels(texture, pixels);
    return pixels;
}

void ImageHandle::DrawScaledImage(float destX, float destY, float destW, float destH,
                                  const ImageHandle& srcImg,
                                  float srcX,  float srcY,  float srcW,  float srcH,
                                  float alpha,
                                  const ImageHandle* pMaskImg,
                                  float maskX, float maskY, float maskW, float maskH,
                                  const ImageHandle* pPaletteImg,
                                  float paletteX, float paletteY, float paletteW) const
{
    uint32_t maskTexture    = 0;
    uint32_t paletteTexture = 0;

    if (pMaskImg)
        maskTexture = pMaskImg->texture;

    if (pPaletteImg)
        paletteTexture = pPaletteImg->texture;

    graphics->DrawTexture(texture, destX, destY, destW, destH,
                          srcImg.texture, srcX, srcY, srcW, srcH, alpha,
                          maskTexture, maskX, maskY, maskW, maskH,
                          paletteTexture, paletteX, paletteY, paletteW);
    return;
}

void ImageHandle::DrawTriangle(float x1, float y1, const Color& c1,
                               float x2, float y2, const Color& c2,
                               float x3, float y3, const Color& c3) const
{
    graphics->DrawTriangle(texture,
                           x1, y1, c1,
                           x2, y2, c2,
                           x3, y3, c3);
    return;
}

void ImageHandle::DrawTriangle(float x1, float y1,
                               float x2, float y2,
                               float x3, float y3,
                               const Color& c) const
{
    graphics->DrawTriangle(texture,
                           x1, y1, c,
                           x2, y2, c,
                           x3, y3, c);
    return;
}

void ImageHandle::DrawRectangle(float x, float y, float width, float height, const Color& c) const
{
    graphics->DrawTriangle(texture,
                           x,       y,        c,
                           x+width, y,        c,
                           x+width, y+height, c);

    graphics->DrawTriangle(texture,
                           x,       y,        c,
                           x,       y+height, c,
                           x+width, y+height, c);
    return;
}

void ImageHandle::DrawCircle(float cx, float cy, float radius, const Color& c) const
{
    if (radius < 0)
        radius = -radius;

    double xOld;
    double yOld;
    double xNew = cx;
    double yNew = static_cast<double>(cy)-radius;

    // Rough heuristic to determine the resolution of the circle.
    long numSegments = static_cast<long>(6*sqrt(radius));
    if (numSegments < 12)
        numSegments = 12;

    for (long n=numSegments-1; n>=0; n--)
    {
        xOld = xNew;
        yOld = yNew;
        xNew = cx-radius*sin(2*PI*n/numSegments);
        yNew = cy-radius*cos(2*PI*n/numSegments);

        graphics->DrawTriangle(texture,
            static_cast<float>(cx),   static_cast<float>(cy),   c,
            static_cast<float>(xOld), static_cast<float>(yOld), c,
            static_cast<float>(xNew), static_cast<float>(yNew), c);
    }

    return;
}

void ImageHandle::DrawLine(float x1, float y1, float x2, float y2, 
                           float thickness, const Color& c) const
{
    if (thickness < 0)
        thickness = -thickness;

    //Calculate perpendicular vector.
    double vx = static_cast<double>(y2)-y1;
    double vy = static_cast<double>(x1)-x2;
    double n = sqrt(vx*vx+vy*vy);

    if (n < 0.01)
    {
        // Too close together.
        DrawCircle(x1, y1, thickness/2, c);
        return;
    }

    // Change the length of the vector to be thickness/2.
    vx *= thickness/(2*n);
    vy *= thickness/(2*n);

    // Draw the main body of the line.
    graphics->DrawTriangle(texture,
        static_cast<float>(x1+vx), static_cast<float>(y1+vy), c,
        static_cast<float>(x1-vx), static_cast<float>(y1-vy), c,
        static_cast<float>(x2+vx), static_cast<float>(y2+vy), c);

    graphics->DrawTriangle(texture,
        static_cast<float>(x2-vx), static_cast<float>(y2-vy), c,
        static_cast<float>(x1-vx), static_cast<float>(y1-vy), c,
        static_cast<float>(x2+vx), static_cast<float>(y2+vy), c);

    // Draw circular end caps.

    // We still draw a full circle but sometimes the sectors are centered at (x1, y1),
    // and sometimes they are centered at (x2, y2).
    double radius = thickness/2;
    double xOld;
    double yOld;
    long   sideOld;
    double xNew =  0;
    double yNew = -1;
    long   sideNew = (vx>0)?1:2;

    // Rough heuristic to determine the resolution of the circle.
    long numSegments = static_cast<long>(6*sqrt(radius));
    if (numSegments < 12)
        numSegments = 12;

    for (long n=numSegments-1; n>=0; n--)
    {
        xOld = xNew;
        yOld = yNew;
        sideOld = sideNew;

        xNew = -sin(2*PI*n/numSegments);
        yNew = -cos(2*PI*n/numSegments);
        sideNew = (xNew*vy-yNew*vx>0)?1:2;

        if (sideNew==1 && sideOld==1)
        {
            graphics->DrawTriangle(texture,
                static_cast<float>(x1),             static_cast<float>(y1),             c,
                static_cast<float>(x1+radius*xOld), static_cast<float>(y1+radius*yOld), c,
                static_cast<float>(x1+radius*xNew), static_cast<float>(y1+radius*yNew), c);
        }

        if (sideNew==1 && sideOld==2)
        {
            graphics->DrawTriangle(texture,
                static_cast<float>(x1),             static_cast<float>(y1),             c,
                static_cast<float>(x1-vx),          static_cast<float>(y1-vy),          c,
                static_cast<float>(x1+radius*xNew), static_cast<float>(y1+radius*yNew), c);

            graphics->DrawTriangle(texture,
                static_cast<float>(x2),             static_cast<float>(y2),             c,
                static_cast<float>(x2-vx),          static_cast<float>(y2-vy),          c,
                static_cast<float>(x2+radius*xOld), static_cast<float>(y2+radius*yOld), c);
        }

        if (sideNew==2 && sideOld==1)
        {
            graphics->DrawTriangle(texture,
                static_cast<float>(x1),             static_cast<float>(y1),             c,
                static_cast<float>(x1+vx),          static_cast<float>(y1+vy),          c,
                static_cast<float>(x1+radius*xOld), static_cast<float>(y1+radius*yOld), c);

            graphics->DrawTriangle(texture,
                static_cast<float>(x2),             static_cast<float>(y2),             c,
                static_cast<float>(x2+vx),          static_cast<float>(y2+vy),          c,
                static_cast<float>(x2+radius*xNew), static_cast<float>(y2+radius*yNew), c);
        }
        
        if (sideNew==2 && sideOld==2)
        {
            graphics->DrawTriangle(texture,
                static_cast<float>(x2),             static_cast<float>(y2),             c,
                static_cast<float>(x2+radius*xOld), static_cast<float>(y2+radius*yOld), c,
                static_cast<float>(x2+radius*xNew), static_cast<float>(y2+radius*yNew), c);
        }
    }

    return;
}

} // End of namespace mi.