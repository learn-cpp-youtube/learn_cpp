#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mi
{

class Graphics; // Forward declare.

class TextureCache
{
public:
    TextureCache();
    ~TextureCache();

    // Size must be greater than 0.
    void Init(Graphics* g, std::uint32_t size);
    void Free();

    void Add(const std::uint32_t* textures, size_t size);
    void Remove(std::uint32_t texture);
    void RemoveAll();

    // Returns 0 if not cached.
    std::uint32_t GetTextureUnit(std::uint32_t texture);

private:
    class CacheData
    {
    public:
        std::uint32_t textureUnit;
        std::uint32_t older;
        std::uint32_t newer;
    };

    std::uint32_t oldest;
    std::uint32_t newest;
    std::unordered_map<std::uint32_t, CacheData> order;
    std::vector<std::uint32_t> unused;

    std::uint32_t placeholderTexture;
    Graphics* g;

    void Display(); // For debug purposes.
};

} // End of namespace mi.
