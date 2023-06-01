#include "TextureCache.h"
#include "Graphics.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <iostream>

using error = std::runtime_error;
using std::uint32_t;
using std::unordered_map;
using std::vector;

namespace mi
{

TextureCache::TextureCache() : g{nullptr}, placeholderTexture{0}
{
}

TextureCache::~TextureCache()
{
    Free();
    return;
}

void TextureCache::Init(Graphics* g, uint32_t size)
{
    Free();

    if (size <= 0)
        throw error("Texture cache size must be greater than 0");

    this->g = g;

    // Create a placeholder image
    // (for when we haven't loaded up the maximum number of textures).
    placeholderTexture = g->CreateTexture(4, 4, nullptr, false);
    g->ClearTexture(placeholderTexture, Color(255, 0, 0));

    // Fill in the cache data.
    oldest = 0;
    newest = 0;

    // Texture unit 0 is reserved for screen blitting.
    // Fill units 1, ..., size with the placeholder image.
    for (uint32_t i=1; i<=size; i++)
    {
        unused.push_back(i);
        g->UseTexture(i, placeholderTexture);
    }

    return;
}

void TextureCache::Free()
{
    order.clear();
    unused.clear();

    if (g)
        g->DeleteTexture(placeholderTexture);

    placeholderTexture = 0;
    g = nullptr;

    return;
}

void TextureCache::Add(const std::uint32_t* textures, size_t size)
{
    unordered_map<uint32_t, CacheData>::iterator it;
    CacheData data;
    
    // Check if the texture is already there first.
    for (size_t i=0; i<size; i++)
    {
        it = order.find(textures[i]);
        if (it == order.end())
            continue; // Not in the map "order".

        if (textures[i] == newest)
            continue; // Already the newest.

        // Remove textures[i] from the order.
        if (it->second.older == 0)
        {
            oldest = it->second.newer;
            order[oldest].older = 0;
        }
        else
        {
            order[it->second.older].newer = it->second.newer;
            order[it->second.newer].older = it->second.older;
        }

        // Add textures[i] back as the newest texture.
        order[newest].newer      = textures[i];
        order[textures[i]].older = newest;
        order[textures[i]].newer = 0;
        newest                   = textures[i];
    }

    // Go through the textures taking any free slots or replacing the oldest texture.
    for (size_t i=0; i<size; i++)
    {
        if (order.find(textures[i]) != order.end())
            continue; // Already in the map "order".

        // Check if there is an unused slot.
        if (unused.size() != 0)
        {
            // Use an unused slot.
            data.textureUnit = unused.back();

            // Add texture to texture unit.
            g->UseTextureUnit(data.textureUnit);
            g->UseTexture(data.textureUnit, textures[i]);
            
            unused.pop_back();
        }
        else
        {
            // No unused slots.

            // Flush drawing before replacing the textures.
            g->FlushRenderData();

            // Use the oldest textureUnit.
            data.textureUnit = order[oldest].textureUnit;

            // Add texture to texture unit.
            g->UseTextureUnit(data.textureUnit);
            g->UseTexture(data.textureUnit, textures[i]);

            // Determine the new oldest and update order.
            uint32_t prevOldest = oldest;
            oldest = order[prevOldest].newer;
            
            if (oldest == 0)
                newest = 0;
            else
                order[oldest].older = 0;

            order.erase(prevOldest);
        }

        // Update order and newest.
        data.newer = 0;
        data.older = newest;
        order[textures[i]] = data;
        
        if (newest == 0)
            oldest = textures[i];
        else
            order[newest].newer = textures[i];

        newest = textures[i];
    }

    return;
}

void TextureCache::Remove(uint32_t texture)
{
    auto it = order.find(texture);
    if (it == order.end())
        return; // Not in the map "order".

    // Flush drawing before removing the texture.
    g->FlushRenderData();

    // Replace texture with placeholder.
    g->UseTextureUnit(it->second.textureUnit);
    g->UseTexture(it->second.textureUnit, placeholderTexture);

    unused.push_back(it->second.textureUnit);

    // Remove texture from the order.
    if (it->second.newer == 0)
    {
        if (it->second.older == 0)
        {
            oldest = 0;
            newest = 0;
        }
        else
        {
            newest = it->second.older;
            order[newest].newer = 0;
        }
    }
    else
    {
        if (it->second.older == 0)
        {
            oldest = it->second.newer;
            order[oldest].older = 0;
        }
        else
        {
            order[it->second.older].newer = it->second.newer;
            order[it->second.newer].older = it->second.older;
        }
    }

    order.erase(texture);
    return;
}

void TextureCache::RemoveAll()
{
    while(newest != 0)
        Remove(newest);

    return;
}

uint32_t TextureCache::GetTextureUnit(uint32_t texture)
{
    auto it = order.find(texture);

    if (it == order.end())
        return 0;

    return it->second.textureUnit;
}

void TextureCache::Display()
{
    // List the textures and their texture units.
    if (oldest == 0)
        std::cout << "<Empty> ";
    else
    {
        uint32_t current = oldest;
        while (current != 0)
        {
            std::cout << order[current].textureUnit << "(" << current << ") ";
            current = order[current].newer;
        }
    }

    // List the unused texture units.
    std::cout << ":";
    if (unused.size() == 0)
        std::cout << " <Empty>";
    else
    {
        // List in reverse order because the back of the list is the first to get used.
        for (size_t i=0; i<unused.size(); i++)
            std::cout << " " << unused[unused.size()-1-i];
    }
    std::cout << std::endl;

    return;
}

} // End of namespace mi.

