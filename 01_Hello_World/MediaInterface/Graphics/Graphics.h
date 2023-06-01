#pragma once

#include "Glad/glad.h"
#include "Color.h"
#include "TextureCache.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

namespace mi
{

class Graphics
{
public:
    Graphics();
    ~Graphics();

    // Methods.
    void          Free();
    void          Init(std::uint32_t screenWidth, std::uint32_t screenHeight);

    std::string   GLenumToString(GLenum e);
    void          CheckGlErrors(const std::string& prefix);
    void          ClearGlErrorQueue();

    void          SetScreenSize(std::uint32_t width, std::uint32_t height);
    void          SetViewport(std::uint32_t width, std::uint32_t height);

    std::uint32_t CreateFramebuffer();
    void          DeleteFramebuffer(std::uint32_t framebuffer);
    // A value of 0 sets the default framebuffer.
    void          UseFramebuffer(std::uint32_t framebuffer);
    // A texture value of 0 unsets the attached texture.
    void          AttachTexture(std::uint32_t framebuffer, std::uint32_t texture);

    std::uint32_t ShaderFromString(const std::string& source, GLenum shaderType);
    std::uint32_t ShaderFromFile(const std::string& filename, GLenum shaderType);
    void          DeleteShader(std::uint32_t shader);

    std::uint32_t CreateShaderProgram();
    void          DeleteShaderProgram(std::uint32_t shaderProgram);
    void          LinkShaderProgram(std::uint32_t shaderProgram);
    // A value of 0 unsets the shader program.
    // Only call after linking.
    void          UseShaderProgram(std::uint32_t shaderProgram);
    void          SetUniform(std::uint32_t shaderProgram, const std::string& name,
                             std::int32_t value);
    // Expects values to be an array of size 4.
    void          SetUniform(std::uint32_t shaderProgram, const std::string& name,
                             const std::int32_t* values);
    void          AttachShader(std::uint32_t shaderProgram, std::uint32_t shader);
    void          DetachShader(std::uint32_t shaderProgram, std::uint32_t shader);

    std::uint32_t CreateVertexArray();
    void          DeleteVertexArray(std::uint32_t vertexArray);
    // A value of 0 unsets the vertex array.
    void          UseVertexArray(std::uint32_t vertexArray);

    std::uint32_t CreateBuffer();
    void          DeleteBuffer(std::uint32_t buffer);
    // A value of 0 unsets the array buffer.
    void          UseArrayBuffer(std::uint32_t buffer);
    void          SetArrayData(std::uint32_t buffer, const void* data, size_t numBytes);
    void          SetAttributeArray(std::uint32_t vertexArray, std::uint32_t attribute,
                                    std::uint32_t buffer, GLenum type, std::uint8_t size,
                                    std::uint32_t offset, std::uint32_t stride);
    void          EnableAttribute(std::uint32_t vertexArray, std::uint32_t attribute,
                                  bool enable = true);

    std::uint32_t CreateTexture(std::uint32_t width, std::uint32_t height,
                                const void* data, bool smooth);
    void          DeleteTexture(std::uint32_t texture);
    void          UseTextureUnit(std::uint32_t textureUnit);
    // A value of 0 unsets the texture.
    void          UseTexture(std::uint32_t textureUnit, std::uint32_t texture);

    std::uint32_t GetTextureWidth(std::uint32_t texture) const;
    std::uint32_t GetTextureHeight(std::uint32_t texture) const;
    bool          GetTextureSmoothFlag(std::uint32_t texture) const;
    void          GetTexturePixels(std::uint32_t texture, std::vector<std::uint8_t>& pixels);
    const std::uint32_t& GetTextureRefCount(std::uint32_t texture) const;
    std::uint32_t&       GetTextureRefCount(std::uint32_t texture);

    void          ClearTexture(std::uint32_t texture, const Color& c);
    void          DisplayTexture(std::uint32_t texture);
    std::uint32_t AddTransparency(std::uint32_t texture, const Color& keyColor, bool smooth);
    std::uint32_t AddTransparency(std::uint32_t colorTexture, std::uint32_t alphaTexture,
                                  bool smooth);
    // Done by the CPU so is slow.
    std::uint32_t CreatePalettedTexture(std::uint32_t texture,
                                        std::uint32_t palette,
                                        std::uint32_t paletteX, std::uint32_t paletteY,
                                        std::uint32_t paletteW, std::uint32_t paletteH,
                                        bool smooth);
    // A maskTexture value of 0 indicates no mask.
    // A paletteTexture value of 0 indicates no palette.
    void          DrawTexture(std::uint32_t destTexture,
                              float destX, float destY, float destW, float destH,
                              std::uint32_t srcTexture,
                              float srcX, float srcY, float srcW, float srcH,
                              float alpha,
                              std::uint32_t maskTexture,
                              float maskX, float maskY, float maskW, float maskH,
                              std::uint32_t paletteTexture,
                              float paletteX, float paletteY, float paletteW);
    void          DrawTriangle(std::uint32_t destTexture,
                               float x1, float y1, const Color& c1,
                               float x2, float y2, const Color& c2,
                               float x3, float y3, const Color& c3);

    void          UpdateTextureCache(const std::uint32_t* textures, size_t size);
    void          ClearTextureCache();

    void          FlushRenderData();

private:
    // Data.
    bool          gladInitialized = false;
    std::uint32_t screenWidth  = 0;
    std::uint32_t screenHeight = 0;
    std::uint32_t screenShaderProgram  = 0; // For blitting image to the entire screen.
    std::uint32_t fillImageVertexArray = 0; // For blitting image to the entire screen.
    std::uint32_t maskShaderProgram    = 0;
    std::uint32_t alphaShaderProgram   = 0;
    std::uint32_t drawFramebuffer      = 0; // For rendering on to a texture/Image.
    std::uint32_t generalRenderShaderProgram = 0;
    std::uint32_t generalRenderVertexArray   = 0;
    std::uint32_t generalRenderBuffer        = 0;
    std::vector<float> generalRenderData;

    std::uint32_t viewportWidth  = 0;
    std::uint32_t viewportHeight = 0;

    std::uint32_t framebufferInUse   = 0;
    std::uint32_t shaderProgramInUse = 0;
    std::uint32_t vertexArrayInUse   = 0;
    std::uint32_t arrayBufferInUse   = 0;
    std::uint32_t textureUnitInUse   = 0;
    std::unordered_map<std::uint32_t, std::uint32_t> texturesInUse;

    class TextureData
    {
    public:
        TextureData(std::uint32_t width=0, std::uint32_t height=0, bool smooth=false)
            : width{width}, height{height}, smooth{smooth}, refCount{0} {}

        std::uint32_t width;
        std::uint32_t height;
        bool          smooth;

        std::uint32_t refCount;
    };

    std::unordered_set<std::uint32_t> buffers;
    std::unordered_set<std::uint32_t> vertexArrays;
    std::unordered_set<std::uint32_t> shaders;
    // shaderPrograms have a set of attached shaders associated with them.
    std::unordered_map<std::uint32_t, std::unordered_set<std::uint32_t>> shaderPrograms;
    // textures have associated data, namely the width and height of the image.
    std::unordered_map<std::uint32_t, TextureData> textures;
    // framebuffers have a texture associated with them.
    std::unordered_map<std::uint32_t, std::uint32_t> framebuffers;

    // Has to be defined after textures (it's destructor requires textures to exist).
    TextureCache textureCache;

    friend class TextureCache;
};

} // End of namespace mi.
