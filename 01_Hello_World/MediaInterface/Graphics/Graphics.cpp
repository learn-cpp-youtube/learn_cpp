#include "Graphics.h"
#include <SDL.h>
#include "ShaderLibrary.h"
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_set>
#include <map>
#include <vector>
#include <cstdint>
#include <unordered_map>

using error = std::runtime_error;
using std::string;
using std::to_string;
using std::stringstream;
using std::ifstream;
using std::unordered_set;
using std::map;
using std::vector;
using std::int32_t;
using std::uint32_t;
using std::uint8_t;
using std::unordered_map;

namespace mi
{

Graphics::Graphics()
{
    gladInitialized    = false;
    framebufferInUse   = 0;
    shaderProgramInUse = 0;
    vertexArrayInUse   = 0;
    arrayBufferInUse   = 0;
    textureUnitInUse   = 0;

    return;
}

Graphics::~Graphics()
{
    Free();
    return;
}

void Graphics::Free()
{
    // This function should not throw an exception
    // as it is called in the destructor.

    // Unbind any OpenGL objects.
    if (gladInitialized)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        for (const auto& element : texturesInUse)
        {
            glActiveTexture(GL_TEXTURE0+element.first);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    framebufferInUse   = 0;
    shaderProgramInUse = 0;
    vertexArrayInUse   = 0;
    arrayBufferInUse   = 0;
    textureUnitInUse   = 0;
    texturesInUse.clear();

    // Cleanup OpenGL framebuffers.
    for (const auto& element : framebuffers)
        glDeleteFramebuffers(1, &element.first);
    framebuffers.clear();

    // Cleanup the OpenGL textures.
    for (const auto& element : textures)
        glDeleteTextures(1, &element.first);
    textures.clear();

    // Cleanup the OpenGL vertex arrays and buffers.
    for (uint32_t vertexArray : vertexArrays)
        glDeleteVertexArrays(1, &vertexArray);
    vertexArrays.clear();

    for (uint32_t buffer : buffers)
        glDeleteBuffers(1, &buffer);
    buffers.clear();

    // Cleanup the OpenGL shader programs.
    for (const auto& p : shaderPrograms)
    {
        for (uint32_t shaderId : p.second)
            glDetachShader(p.first, shaderId);

        glDeleteProgram(p.first);
    }
    shaderPrograms.clear();

    for (uint32_t shaderId : shaders)
        glDeleteShader(shaderId);
    shaders.clear();

    // Reset the textureCache.
    textureCache.Free();

    //Cleanup the OpenGL error queue.
    if (gladInitialized)
        ClearGlErrorQueue();

    // Clear the render data.
    generalRenderData.clear();

    gladInitialized = false;
    return;
}

void Graphics::Init(uint32_t screenWidth, uint32_t screenHeight)
{
    // Start by doing a cleanup (just in case).
    Free();

    // Initialize GLAD so we can use the latest OpenGL features.

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        throw error("Initializing GLAD failed.");
    }

    CheckGlErrors("Initializing GLAD");
    gladInitialized = true;

    // Update texturesInUse.
    textureUnitInUse = 0;
    texturesInUse[textureUnitInUse] = 0;

    // Update framebuffersInUse.
    framebufferInUse = 0;
    framebuffers[framebufferInUse] = 0;

    // Store the screen dimensions.
    this->screenWidth  = screenWidth;
    this->screenHeight = screenHeight;

    // Set the viewport.
    viewportWidth  = screenWidth;
    viewportHeight = screenHeight;
    glViewport(0, 0, viewportWidth, viewportHeight);
    CheckGlErrors("Initializing viewport");

    // Enable alpha blending.
    glEnable(GL_BLEND);
    CheckGlErrors("Initializing alpha blending to enabled");

    // Assume all images have their color channels premultiplied by the alpha channel.
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    CheckGlErrors("Setting blending function");

    // Create a vertex array for blitting an image to the screen.
    fillImageVertexArray = CreateVertexArray();
    UseVertexArray(fillImageVertexArray);
    EnableAttribute(fillImageVertexArray, 0);
    EnableAttribute(fillImageVertexArray, 1);

    // Fill the vertex array with vertex coordinates, and texture coordinates
    // (both of which are represented by quad[]).
    float quad[8] =
        {
             0, 1,
             1, 1,
             1, 0,
             0, 0
        };
    uint32_t buffer = CreateBuffer();
    UseArrayBuffer(buffer);
    const long sf = sizeof(float); // To save typing.
    SetAttributeArray(fillImageVertexArray, 0, buffer, GL_FLOAT, 2, 0, 2*sf);
    SetAttributeArray(fillImageVertexArray, 1, buffer, GL_FLOAT, 2, 0, 2*sf);
    SetArrayData(buffer, quad, 10*sizeof(float));

    // Create a shader program to blit the texture in textureUnit 0 to the screen.
    screenShaderProgram = CreateShaderProgram();
    
    AttachShader(screenShaderProgram,
        ShaderFromString(shader::vertex::blitToScreen, GL_VERTEX_SHADER));
    AttachShader(screenShaderProgram,
        ShaderFromString(shader::fragment::blitToScreen, GL_FRAGMENT_SHADER));
    LinkShaderProgram(screenShaderProgram);

    // Reserve texture unit 0 specifically for blitting to the screen.
    SetUniform(screenShaderProgram, "tex", 0);

    // Create a framebuffer to allow us to draw on textures.
    drawFramebuffer = CreateFramebuffer(); 

    // Create shader programs to create alpha channels.
    maskShaderProgram = CreateShaderProgram();

    AttachShader(maskShaderProgram,
        ShaderFromString(shader::vertex::blitToImage, GL_VERTEX_SHADER));
    AttachShader(maskShaderProgram,
        ShaderFromString(shader::fragment::createMask, GL_FRAGMENT_SHADER));
    LinkShaderProgram(maskShaderProgram);
    
    int32_t matchColor[4]   = {0, 0, 0, 255};
    int32_t noMatchColor[4] = {255, 255, 255, 255};
    SetUniform(maskShaderProgram, "matchColor",   matchColor);
    SetUniform(maskShaderProgram, "noMatchColor", noMatchColor);
    SetUniform(maskShaderProgram, "tex", 1);

    alphaShaderProgram = CreateShaderProgram();

    AttachShader(alphaShaderProgram,
        ShaderFromString(shader::vertex::blitToImage, GL_VERTEX_SHADER));
    AttachShader(alphaShaderProgram,
        ShaderFromString(shader::fragment::addAlpha, GL_FRAGMENT_SHADER));
    LinkShaderProgram(alphaShaderProgram);
    
    SetUniform(alphaShaderProgram, "texColor", 1);
    SetUniform(alphaShaderProgram, "texAlpha", 2);

    // Create a vertex array for general rendering of 2D images.
    generalRenderVertexArray = CreateVertexArray();
    UseVertexArray(generalRenderVertexArray);
    EnableAttribute(generalRenderVertexArray, 0); // Position.
    EnableAttribute(generalRenderVertexArray, 1); // Type (use texture unit, or color).
    EnableAttribute(generalRenderVertexArray, 2); // Color or texture coordinates with alpha.
    EnableAttribute(generalRenderVertexArray, 3); // Mask Type (use texture unit, or 0=None).
    EnableAttribute(generalRenderVertexArray, 4); // Mask texture coordinates.
    EnableAttribute(generalRenderVertexArray, 5); // Palette Type (use texture unit, or 0=None).
    EnableAttribute(generalRenderVertexArray, 6); // Palette texture coordinates.
    EnableAttribute(generalRenderVertexArray, 7); // Palette texture width.

    // Create a buffer to hold the data.
    generalRenderBuffer = CreateBuffer();
    UseArrayBuffer(generalRenderBuffer);
    // Structure:
    //  0 (attribute 0) position x
    //  1 (attribute 0) position y
    //  2 (attribute 1) type  (texture unit id, or 0 for color)
    //  3 (attribute 2) red   (if type is color), texture x coordinate (if type is texture)
    //  4 (attribute 2) green (if type is color), texture y coordinate (if type is texture)
    //  5 (attribute 2) blue  (if type is color), 0                    (if type is texture)
    //  6 (attribute 2) alpha
    //  7 (attribute 3) mask type (texture unit id, or 0 for no mask)
    //  8 (attribute 4) mask texture x coordinate
    //  9 (attribute 4) mask texture y coordinate
    // 10 (attribute 5) palette type (texture unit id, or 0 for no palette)
    // 11 (attribute 6) palette texture x coordinate
    // 12 (attribute 6) palette texture y coordinate
    // 13 (attribute 7) palette width
    SetAttributeArray(generalRenderVertexArray, 0, generalRenderBuffer, GL_FLOAT, 2,  0*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 1, generalRenderBuffer, GL_FLOAT, 1,  2*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 2, generalRenderBuffer, GL_FLOAT, 4,  3*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 3, generalRenderBuffer, GL_FLOAT, 1,  7*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 4, generalRenderBuffer, GL_FLOAT, 2,  8*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 5, generalRenderBuffer, GL_FLOAT, 1, 10*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 6, generalRenderBuffer, GL_FLOAT, 2, 11*sf, 14*sf);
    SetAttributeArray(generalRenderVertexArray, 7, generalRenderBuffer, GL_FLOAT, 1, 13*sf, 14*sf);

    // Create a shader program for general rendering of 2D images.
    generalRenderShaderProgram = CreateShaderProgram();

    AttachShader(generalRenderShaderProgram,
        ShaderFromString(shader::vertex::drawToImage, GL_VERTEX_SHADER));
    AttachShader(generalRenderShaderProgram,
        ShaderFromString(shader::fragment::drawToImage, GL_FRAGMENT_SHADER));
    LinkShaderProgram(generalRenderShaderProgram);

    const long numCachedTextures = 8;
    for (long i=1; i<=numCachedTextures; i++)
        SetUniform(generalRenderShaderProgram, "tex"+to_string(i), i);

    // Initialize texture cache object.
    textureCache.Init(this, numCachedTextures);

    return;
}

string Graphics::GLenumToString(GLenum e)
{
    static const map<GLenum, string> mapping =
        {
            // Error return values.
            {GL_NO_ERROR,                      "GL_NO_ERROR"},
            {GL_INVALID_ENUM,                  "GL_INVALID_ENUM"},
            {GL_INVALID_VALUE,                 "GL_INVALID_VALUE"},
            {GL_INVALID_OPERATION,             "GL_INVALID_OPERATION"},
            {GL_OUT_OF_MEMORY,                 "GL_OUT_OF_MEMORY"},
            {GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"},

            // Numerical types.
            {GL_BYTE,                          "GL_BYTE"},
            {GL_UNSIGNED_BYTE,                 "GL_UNSIGNED_BYTE"},
            {GL_SHORT,                         "GL_SHORT"},
            {GL_UNSIGNED_SHORT,                "GL_UNSIGNED_SHORT"},
            {GL_INT,                           "GL_INT"},
            {GL_UNSIGNED_INT,                  "GL_UNSIGNED_INT"},
            {GL_HALF_FLOAT,                    "GL_HALF_FLOAT"},
            {GL_FLOAT,                         "GL_FLOAT"},
            {GL_DOUBLE,                        "GL_DOUBLE"},

            // Shader types.
            {GL_VERTEX_SHADER,                 "GL_VERTEX_SHADER"},
            {GL_GEOMETRY_SHADER,               "GL_GEOMETRY_SHADER"},
            {GL_FRAGMENT_SHADER,               "GL_FRAGMENT_SHADER"}
        };

    const auto it = mapping.find(e);
    if (it == mapping.end())
        return "Unknown("+to_string(e)+")";

    return it->second;
}

void Graphics::CheckGlErrors(const string& prefix)
{
    GLenum err;
    string errList = "";

    err = glGetError();
    while (err != GL_NO_ERROR)
    {
        // Add err to the list of errors.
        if (errList != "")
            errList += ", ";

        errList += GLenumToString(err);

        // Check if there are any more errors.
        err = glGetError();
    }

    if (errList.size() == 0)
        return; // No errors.

    errList = "OpenGL error: "+errList;
    if (prefix != "")
        errList = prefix+": "+errList;

    throw error(errList);
}

void Graphics::ClearGlErrorQueue()
{
    GLenum err = glGetError();
    while (err != GL_NO_ERROR)
        err = glGetError();

    return;
}

void Graphics::SetScreenSize(uint32_t width, uint32_t height)
{
    screenWidth  = width;
    screenHeight = height;
    return; 
}

void Graphics::SetViewport(uint32_t width, uint32_t height)
{
    if (viewportWidth == width && viewportHeight == height)
        return;

    glViewport(0, 0, width, height);
    CheckGlErrors("Setting the viewport");

    viewportWidth  = width;
    viewportHeight = height;
    return;
}

uint32_t Graphics::CreateFramebuffer()
{
    uint32_t framebuffer = 0;
    uint32_t oldFramebuffer = framebufferInUse;

    try
    {
        glGenFramebuffers(1, &framebuffer);
        CheckGlErrors("Creating a framebuffer");
        framebuffers[framebuffer] = 0;

        UseFramebuffer(framebuffer);

        GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);
        CheckGlErrors("Setting the framebuffer draw buffer");

        UseFramebuffer(oldFramebuffer);
    }
    catch(...)
    {
        UseFramebuffer(oldFramebuffer);
        DeleteFramebuffer(framebuffer);
        throw;            
    }

    return framebuffer;
}

void Graphics::DeleteFramebuffer(uint32_t framebuffer)
{
    if (framebuffer == 0)
        return; // Don't delete the default framebuffer.

    auto it = framebuffers.find(framebuffer);

    if (it == framebuffers.end())
        return; // Already deleted.

    if (framebufferInUse == framebuffer)
        UseFramebuffer(0);

    glDeleteFramebuffers(1, &framebuffer);
    CheckGlErrors("Deleting framebuffer");

    framebuffers.erase(framebuffer);
    return;
}

void Graphics::UseFramebuffer(uint32_t framebuffer)
{
    if (framebuffer == framebufferInUse)
        return; // Already being used.

    // If framebuffer is 0 we are using the default framebuffer.

    if (framebuffer)
    {
        if (framebuffers.find(framebuffer) == framebuffers.end())
            throw error("Trying to use an unknown OpenGL framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    CheckGlErrors("Setting framebuffer to use");

    framebufferInUse = framebuffer;
    return;
}

void Graphics::AttachTexture(uint32_t framebuffer, uint32_t texture)
{
    if (framebuffer == 0)
    {
        if (texture != 0)
            throw error("Trying to attach a texture to the default OpenGL framebuffer");
        return;
    }

    auto it = framebuffers.find(framebuffer);
    if (it != framebuffers.end() && it->second == texture)
        return; // Already attached.

    uint32_t oldFramebuffer = framebufferInUse;

    try
    {
        UseFramebuffer(framebuffer);

        if (texture && textures.find(texture) == textures.end())
            throw error("Trying to attach an unknown OpenGL texture");

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        CheckGlErrors("Attaching texture to framebuffer");
        framebuffers[framebuffer] = texture;

        if (texture)
        {
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw error("Incomplete framebuffer");
            CheckGlErrors("Checking framebuffer status");
        }

        UseFramebuffer(oldFramebuffer);
    }
    catch(...)
    {
        UseFramebuffer(oldFramebuffer);
        throw;
    }

    return;
}

uint32_t Graphics::ShaderFromString(const string& source, GLenum shaderType)
{
    uint32_t shader = glCreateShader(shaderType);
 
    if (shader == 0)
        throw error("Creating OpenGL shader failed");

    CheckGlErrors("Creating shader");
    
    shaders.insert(shader);

    const char* srcStr = source.c_str();
    glShaderSource(shader, 1, &srcStr, nullptr);
    CheckGlErrors("Setting shader source code");

    glCompileShader(shader);
    CheckGlErrors("Compiling shader");
 
    int32_t returnValue;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &returnValue);
    CheckGlErrors("Checking shader compilation status");

    if (returnValue == GL_TRUE)
        return shader;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &returnValue);
    CheckGlErrors("Getting shader info log length");

    vector<char> infoLog(returnValue);
    glGetShaderInfoLog(shader,
                       static_cast<GLsizei>(infoLog.size()),
                       &returnValue,
                       infoLog.data());
    CheckGlErrors("Getting shader info log");

    string err = infoLog.data();
    throw error("OpenGL shader failed to compile: "+err);
}

uint32_t Graphics::ShaderFromFile(const string& filename, GLenum shaderType)
{
    ifstream file(filename);

    if (!file)
        throw error("Couldn't open file "+filename);
 
    stringstream ss;
    ss << file.rdbuf();

    return ShaderFromString(ss.str(), shaderType);
}

void Graphics::DeleteShader(uint32_t shader)
{
    glDeleteShader(shader);
    CheckGlErrors("Deleting shader");
    shaders.erase(shader);
    return;
}

uint32_t Graphics::CreateShaderProgram()
{
    uint32_t shaderProgram = glCreateProgram();

    CheckGlErrors("Creating shader program");

    if (shaderProgram == 0)
        throw error("Creating OpenGL shader program failed");

    shaderPrograms[shaderProgram] = unordered_set<uint32_t>();

    return shaderProgram;
}

void Graphics::DeleteShaderProgram(uint32_t shaderProgram)
{
    auto it = shaderPrograms.find(shaderProgram);

    if (it == shaderPrograms.end())
        return; // Already deleted.

    if (shaderProgramInUse == shaderProgram)
        UseShaderProgram(0);

    // Create a copy of the list of all the attached shaders.
    // This way we avoid iterating through a list we are modifying.
    vector<uint32_t> shaders(it->second.begin(), it->second.end());
    for (uint32_t shader : shaders)
    {
        glDetachShader(shaderProgram, shader);
        CheckGlErrors("Detaching shader before deleting shader program");

        shaderPrograms[shaderProgram].erase(shader);
    }

    glDeleteProgram(shaderProgram);
    CheckGlErrors("Deleting shader program");

    shaderPrograms.erase(shaderProgram);
    return;
}

void Graphics::LinkShaderProgram(uint32_t shaderProgram)
{
    auto it = shaderPrograms.find(shaderProgram);

    if (it == shaderPrograms.end())
        throw error("Trying to link an unknown OpenGL shader program");

    glLinkProgram(shaderProgram);
    CheckGlErrors("Linking shader program");
 
    int32_t returnValue;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &returnValue);
    CheckGlErrors("Checking shader program link status");

    if (returnValue == GL_TRUE)
        return; // Success.

    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &returnValue);
    CheckGlErrors("Getting shader program info log length");

    vector<char> infoLog(returnValue);
    glGetProgramInfoLog(shaderProgram,
                        static_cast<GLsizei>(infoLog.size()),
                        &returnValue,
                        infoLog.data());
    CheckGlErrors("Getting shader program info log");

    string err = infoLog.data();
    throw error("OpenGL program shader failed to link: "+err);
}

void Graphics::UseShaderProgram(uint32_t shaderProgram)
{
    if (shaderProgram == shaderProgramInUse)
        return; // Already being used.

    // If shaderProgram is 0 we are disabling the shader. 

    if (shaderProgram)
    {
        if (shaderPrograms.find(shaderProgram) == shaderPrograms.end())
            throw error("Trying to use an unknown OpenGL shader program");
    }

    glUseProgram(shaderProgram);

    if (shaderProgram)
        CheckGlErrors("Setting shader program to use");
    else
        CheckGlErrors("Unsetting the shader program");

    shaderProgramInUse = shaderProgram;
    return;
}

void Graphics::SetUniform(uint32_t shaderProgram, const string& name, int32_t value)
{
    uint32_t oldShaderProgram = shaderProgramInUse;

    try
    {
        UseShaderProgram(shaderProgram);

        int32_t location = glGetUniformLocation(shaderProgramInUse, name.c_str());
        CheckGlErrors("Getting location of variable in shader");

        glUniform1i(location, value);
        CheckGlErrors("Setting value of variable in shader");

        UseShaderProgram(oldShaderProgram);
    }
    catch(...)
    {
        UseShaderProgram(oldShaderProgram);
        throw;
    }

    return;
}

void Graphics::SetUniform(uint32_t shaderProgram, const string& name, const int32_t* values)
{
    uint32_t oldShaderProgram = shaderProgramInUse;

    try
    {
        UseShaderProgram(shaderProgram);

        int32_t location = glGetUniformLocation(shaderProgramInUse, name.c_str());
        CheckGlErrors("Getting location of array in shader");

        glUniform4iv(location, 1, values);
        CheckGlErrors("Setting value of arrays in shader");

        UseShaderProgram(oldShaderProgram);
    }
    catch(...)
    {
        UseShaderProgram(oldShaderProgram);
        throw;
    }

    return;
}

void Graphics::AttachShader(uint32_t shaderProgram, uint32_t shader)
{
    auto it = shaderPrograms.find(shaderProgram);

    if (it == shaderPrograms.end())
        throw error("Trying to attach an OpenGL shader to an unknown shader program");

    if (it->second.count(shader))
        return; // Already attached.

    glAttachShader(shaderProgram, shader);
    CheckGlErrors("Attaching shader");

    it->second.insert(shader);
    return;
}

void Graphics::DetachShader(uint32_t shaderProgram, uint32_t shader)
{
    auto it = shaderPrograms.find(shaderProgram);

    if (it == shaderPrograms.end())
        throw error("Trying to detach an OpenGL shader from an unknown shader program");

    if (it->second.count(shader) == 0)
        return; // Already detached.

    glDetachShader(shaderProgram, shader);
    CheckGlErrors("Detaching shader");

    it->second.erase(shader);
    return;
}

uint32_t Graphics::CreateVertexArray()
{
    uint32_t vertexArray;

    glGenVertexArrays(1, &vertexArray);
    CheckGlErrors("Creating vertex array");

    vertexArrays.insert(vertexArray);
    return vertexArray;
}

void Graphics::DeleteVertexArray(uint32_t vertexArray)
{
    auto it = vertexArrays.find(vertexArray);

    if (it == vertexArrays.end())
        return; // Already deleted.

    if (vertexArrayInUse == vertexArray)
        UseVertexArray(0);

    glDeleteVertexArrays(1, &vertexArray);
    CheckGlErrors("Deleting vertex array");

    vertexArrays.erase(vertexArray);
    return;
}

void Graphics::UseVertexArray(uint32_t vertexArray)
{
    if (vertexArray == vertexArrayInUse)
        return; // Already being used.

    // If vertexArray is 0 we are disabling the vertexArray.

    if (vertexArray)
    {
        if (vertexArrays.find(vertexArray) == vertexArrays.end())
            throw error("Trying to use an unknown OpenGL vertex array");
    }

    glBindVertexArray(vertexArray);

    if (vertexArray)
        CheckGlErrors("Setting vertex array to use");
    else
        CheckGlErrors("Unsetting the vertex array");

    vertexArrayInUse = vertexArray;
    return;
}

uint32_t Graphics::CreateBuffer()
{
    uint32_t buffer;

    glGenBuffers(1, &buffer);
    CheckGlErrors("Creating a buffer");

    buffers.insert(buffer);
    return buffer;
}

void Graphics::DeleteBuffer(uint32_t buffer)
{
    auto it = buffers.find(buffer);

    if (it == buffers.end())
        return; // Already deleted.

    if (arrayBufferInUse == buffer)
        UseArrayBuffer(0);

    glDeleteBuffers(1, &buffer);
    CheckGlErrors("Deleting buffer");

    buffers.erase(buffer);
    return;
}

void Graphics::UseArrayBuffer(uint32_t buffer)
{
    if (buffer == arrayBufferInUse)
        return; // Already being used.

    // If buffer is 0 we are disabling the array buffer.

    if (buffer)
    {
        if (buffers.find(buffer) == buffers.end())
            throw error("Trying to use an unknown OpenGL buffer");
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    if (buffer)
        CheckGlErrors("Setting array buffer to use");
    else
        CheckGlErrors("Unsetting the array buffer");

    arrayBufferInUse = buffer;
    return;
}

void Graphics::SetArrayData(uint32_t buffer, const void* data, size_t numBytes)
{
    uint32_t oldArrayBuffer = arrayBufferInUse;

    try
    {
        UseArrayBuffer(buffer);

        glBufferData(GL_ARRAY_BUFFER, numBytes, data, GL_DYNAMIC_DRAW);
        CheckGlErrors("Setting the array data");

        UseArrayBuffer(oldArrayBuffer);
    }
    catch(...)
    {
        UseArrayBuffer(oldArrayBuffer);
        throw;
    }
    return;
}

void Graphics::SetAttributeArray(uint32_t vertexArray, uint32_t attribute,
                                 uint32_t buffer, GLenum type, uint8_t size,
                                 uint32_t offset, uint32_t stride)
{
    uint32_t oldVertexArray = vertexArrayInUse;
    uint32_t oldArrayBuffer = arrayBufferInUse;

    try
    {
        UseVertexArray(vertexArray);
        UseArrayBuffer(buffer);

        glVertexAttribPointer(attribute, size, type, GL_FALSE, stride,
                              static_cast<char*>(0)+offset);
        CheckGlErrors("Setting the attribute array");

        UseArrayBuffer(oldArrayBuffer);
        UseVertexArray(oldVertexArray);
    }
    catch(...)
    {
        UseArrayBuffer(oldArrayBuffer);
        UseVertexArray(oldVertexArray);
        throw;
    }
    return;
}

void Graphics::EnableAttribute(uint32_t vertexArray, uint32_t attribute, bool enable)
{
    uint32_t oldVertexArray = vertexArrayInUse;

    try
    {
        UseVertexArray(vertexArray);

        if (enable)
        {
            glEnableVertexAttribArray(attribute);
            CheckGlErrors("Enabling attribute");
        }
        else
        {
            glDisableVertexAttribArray(attribute);
            CheckGlErrors("Disabling attribute");
        }

        UseVertexArray(oldVertexArray);
    }
    catch(...)
    {
        UseVertexArray(oldVertexArray);
        throw;
    }
    return;
}

uint32_t Graphics::CreateTexture(uint32_t width, uint32_t height,
                                 const void* data, bool smooth)
{
    uint32_t texture         = 0;
    uint32_t oldTextureInUse = texturesInUse[textureUnitInUse];

    try
    {
        glGenTextures(1, &texture);
        CheckGlErrors("Creating texture");
        textures[texture] = TextureData();

        UseTexture(textureUnitInUse, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        CheckGlErrors("Creating an image in a texture");
        textures[texture] = TextureData(width, height, smooth);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (smooth?GL_LINEAR:GL_NEAREST));
        CheckGlErrors("Setting texture min filter");

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (smooth?GL_LINEAR:GL_NEAREST));
        CheckGlErrors("Setting texture mag filter");

        UseTexture(textureUnitInUse, oldTextureInUse);
    }
    catch(...)
    {
        UseTexture(textureUnitInUse, oldTextureInUse);
        DeleteTexture(texture);
        throw;
    }

    return texture;
}

void Graphics::DeleteTexture(uint32_t texture)
{
    auto it = textures.find(texture);

    if (it == textures.end())
        return; // Already deleted.

    FlushRenderData(); // Maybe being drawn to, or drawing on something else.

    uint32_t oldTextureUnitInUse = textureUnitInUse;
    try
    {
        // Remove from the texture cache.
        textureCache.Remove(texture);

        // Unbind the texture if it is in use.
        for (const auto& element : texturesInUse)
        {
            if (element.second == texture)
            {
                UseTextureUnit(element.first);
                UseTexture(element.first, 0);
            }
        }

        // Detach texture if it is being used in a framebuffer.
        for (auto& element : framebuffers)
        {
            if (element.second == texture)
                AttachTexture(element.first, 0);
        }
    }
    catch(...)
    {
        UseTextureUnit(oldTextureUnitInUse);
        throw;
    }

    UseTextureUnit(oldTextureUnitInUse);

    glDeleteTextures(1, &texture);
    CheckGlErrors("Deleting texture");

    textures.erase(texture);
    return;
}

void Graphics::UseTextureUnit(uint32_t textureUnit)
{
    if (textureUnit == textureUnitInUse)
        return; // Already being used.

    glActiveTexture(GL_TEXTURE0+textureUnit);
    CheckGlErrors("Setting texture unit to use");

    textureUnitInUse = textureUnit;

    // Update the texture stored in texture unit.
    if (texturesInUse.find(textureUnit) == texturesInUse.end())
        texturesInUse[textureUnit] = 0;

    return;
}

void Graphics::UseTexture(uint32_t textureUnit, uint32_t texture)
{
    auto it = texturesInUse.find(textureUnit);

    if (it != texturesInUse.end() && it->second == texture)
        return; // Already in use.

    // If texture is 0 we are unbinding the texture held in textureUnit.

    if (texture)
    {
        if (textures.find(texture) == textures.end())
            throw error("Trying to use an unknown OpenGL texture");
    }

    uint32_t oldTextureUnitInUse = textureUnitInUse;
    try
    {
        UseTextureUnit(textureUnit);

        glBindTexture(GL_TEXTURE_2D, texture);

        if (texture)
            CheckGlErrors("Setting texture to use");
        else
            CheckGlErrors("Unsetting texture to use");

        texturesInUse[textureUnitInUse] = texture;

        UseTextureUnit(oldTextureUnitInUse);
    }
    catch(...)
    {
        UseTextureUnit(oldTextureUnitInUse);
        throw;
    }

    return;
}

uint32_t Graphics::GetTextureWidth(uint32_t texture) const
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture width");

    return it->second.width;
}

uint32_t Graphics::GetTextureHeight(uint32_t texture) const
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture height");

    return it->second.height;
}

bool Graphics::GetTextureSmoothFlag(uint32_t texture) const
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture smooth flag");

    return it->second.smooth;
}

void Graphics::GetTexturePixels(uint32_t texture, vector<uint8_t>& pixels)
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Getting the pixel data failed because the underlying texture is missing");

    FlushRenderData(); // We may be drawing to this image.

    UseFramebuffer(drawFramebuffer);
    AttachTexture(drawFramebuffer, texture);

    pixels.resize(std::size_t(4) * it->second.width * it->second.height);

    glReadPixels(0, 0, it->second.width, it->second.height, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());
    CheckGlErrors("Geting raw pixel data");

    return;
}

const std::uint32_t& Graphics::GetTextureRefCount(std::uint32_t texture) const
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture reference count");

    return it->second.refCount;
}

std::uint32_t& Graphics::GetTextureRefCount(std::uint32_t texture)
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture reference count");

    return it->second.refCount;
}

void Graphics::ClearTexture(uint32_t texture, const Color& c)
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture to clear");

    FlushRenderData(); // We may be drawing using this image.

    UseFramebuffer(drawFramebuffer);
    AttachTexture(drawFramebuffer, texture);

    SetViewport(it->second.width, it->second.height);
    double alpha = c.a/255.0;
    glClearColor(static_cast<float>(c.r/255.0*alpha),
                 static_cast<float>(c.g/255.0*alpha),
                 static_cast<float>(c.b/255.0*alpha),
                 static_cast<float>(alpha));
    glClear(GL_COLOR_BUFFER_BIT);
    CheckGlErrors("Clearing texture");

    return;
}

void Graphics::DisplayTexture(uint32_t texture)
{
    FlushRenderData(); // We may be drawing using this texture.

    UseFramebuffer(0);
    UseShaderProgram(screenShaderProgram);
    UseVertexArray(fillImageVertexArray);
    UseTexture(0, texture);

    SetViewport(screenWidth, screenHeight);

    // We are drawing over every pixel so we don't need to clear the buffer.
    // We do it anyway as a hack to get around an Intel OpenGL driver issue.
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    CheckGlErrors("Updating screen");

    return;
}

uint32_t Graphics::AddTransparency(uint32_t texture, const Color& keyColor, bool smooth)
{
    auto it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find texture to add transparency");

    uint32_t width  = it->second.width;
    uint32_t height = it->second.height;

    uint32_t output = 0;
    uint32_t mask   = 0;

    // Note that the textures are initialized to 0 if they are not already in the maps.
    // (Which is the behaviour we want.)
    uint32_t oldTexture1 = texturesInUse[1];
    uint32_t oldTexture2 = texturesInUse[2];

    try
    {
        FlushRenderData(); // We may be drawing using this image.

        // Allocate textures.
        output = CreateTexture(width, height, nullptr, smooth);
        mask   = CreateTexture(width, height, nullptr, false);

        // Set the settings which don't change between draws.
        SetViewport(width, height);
        UseFramebuffer(drawFramebuffer);
        UseTextureUnit(1);
        UseTexture(1, texture);
        UseVertexArray(fillImageVertexArray);

        // Create the mask.
        AttachTexture(drawFramebuffer, mask);
        UseShaderProgram(maskShaderProgram);
        int32_t key[4] = {keyColor.r, keyColor.g, keyColor.b, keyColor.a};
        SetUniform(maskShaderProgram, "colorKey", key);
        ClearTexture(mask, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        CheckGlErrors("Creating a mask");

        // Create the alpha channel.
        AttachTexture(drawFramebuffer, output);
        UseShaderProgram(alphaShaderProgram);
        UseTextureUnit(2);
        UseTexture(2, mask);
        ClearTexture(output, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        CheckGlErrors("Creating alpha channel");

        // Cleanup the mask texture as it is no longer used.
        DeleteTexture(mask);

        // Set the textures back.
        UseTexture(1, oldTexture1);
        UseTexture(2, oldTexture2);

        return output;
    }
    catch(...)
    {
        DeleteTexture(mask);
        DeleteTexture(output);
        UseTexture(1, oldTexture1);
        UseTexture(2, oldTexture2);
        throw;
    }  
}

uint32_t Graphics::AddTransparency(uint32_t colorTexture, uint32_t alphaTexture, bool smooth)
{
    auto it = textures.find(colorTexture);

    if (it == textures.end())
        throw error("Couldn't find texture to add transparency");

    uint32_t width  = it->second.width;
    uint32_t height = it->second.height;

    uint32_t output = 0;

    // Note that the textures are initialized to 0 if they are not already in the maps.
    // (Which is the behaviour we want.)
    uint32_t oldTexture1 = texturesInUse[1];
    uint32_t oldTexture2 = texturesInUse[2];

    try
    {
        FlushRenderData(); // We may be drawing using this image.

        // Allocate the output texture.
        output = CreateTexture(width, height, nullptr, smooth);

        // Set the parameters for the alpha shader program.
        UseFramebuffer(drawFramebuffer);
        AttachTexture(drawFramebuffer, output);

        UseTextureUnit(1);
        UseTexture(1, colorTexture);
        UseTextureUnit(2);
        UseTexture(2, alphaTexture);

        SetViewport(width, height);

        UseVertexArray(fillImageVertexArray);
        UseShaderProgram(alphaShaderProgram);

        ClearTexture(output, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        CheckGlErrors("Creating alpha channel");

        // Set the textures back.
        UseTexture(1, oldTexture1);
        UseTexture(2, oldTexture2);

        return output;
    }
    catch(...)
    {
        DeleteTexture(output);
        UseTexture(1, oldTexture1);
        UseTexture(2, oldTexture2);
        throw;
    }  
}

uint32_t Graphics::CreatePalettedTexture(uint32_t texture,
                                         uint32_t palette,
                                         uint32_t paletteX, uint32_t paletteY,
                                         uint32_t paletteW, uint32_t paletteH,
                                         bool smooth)
{
    auto it = textures.find(palette);

    if (it == textures.end())
        throw error("Couldn't find palette texture");

    uint32_t pixelsWidth  = it->second.width;
    uint32_t pixelsHeight = it->second.height;

    vector<uint8_t> pixels;
    GetTexturePixels(palette, pixels);

    // Go through all the pixels in the palette, and create a map of their colors.
    uint32_t    index = 0;
    uint32_t    color;
    std::size_t pixelIndex;
    unordered_map<uint32_t, uint32_t> colorToIndex;

    for (uint32_t y=paletteY; y<paletteY+paletteH; y++)
    for (uint32_t x=paletteX; x<paletteX+paletteW; x++)
    {
        pixelIndex = static_cast<std::size_t>(4)*x + static_cast<std::size_t>(4)*y*pixelsWidth;

        if (pixelIndex+3 >= pixels.size())
            throw error("Palette dimensions are too large");

        color = (pixels[pixelIndex+0] << 24)
              | (pixels[pixelIndex+1] << 16)
              | (pixels[pixelIndex+2] << 8)
              | (pixels[pixelIndex+3] << 0);

        if (colorToIndex.find(color) == colorToIndex.end())
            colorToIndex[color] = (index<<8) | 0xff; // Store the index in RGB, set A = 255.

        index++;
    }

    // Replace all the color data in texture with palette indices.
    it = textures.find(texture);

    if (it == textures.end())
        throw error("Couldn't find image texture to replace with palette indices");

    pixelsWidth  = it->second.width;
    pixelsHeight = it->second.height;

    GetTexturePixels(texture, pixels);

    for (size_t i=0; i<pixels.size(); i+=4)
    {
        color = (pixels[i+0] << 24)
              | (pixels[i+1] << 16)
              | (pixels[i+2] << 8)
              | (pixels[i+3] << 0);

        if (colorToIndex.find(color) == colorToIndex.end())
            color = 0xff;  // Default to index 0.
        else
            color = colorToIndex[color];

        pixels[i+0] = (color >> 24) & 0xff;
        pixels[i+1] = (color >> 16) & 0xff;
        pixels[i+2] = (color >> 8)  & 0xff;
        pixels[i+3] = (color >> 0)  & 0xff;
    }

    return CreateTexture(pixelsWidth, pixelsHeight, pixels.data(), smooth);
}

void Graphics::DrawTexture(uint32_t destTexture,
                           float destX, float destY, float destW, float destH,
                           uint32_t srcTexture,
                           float srcX, float srcY, float srcW, float srcH,
                           float alpha,
                           uint32_t maskTexture,
                           float maskX, float maskY, float maskW, float maskH,
                           uint32_t paletteTexture,
                           float paletteX, float paletteY, float paletteW)
{
    // Update the drawFramebuffer if the destination texture has changed.
    if (destTexture != framebuffers[drawFramebuffer])
    {
        FlushRenderData();
        UseFramebuffer(drawFramebuffer);
        AttachTexture(drawFramebuffer, destTexture);
    }

    // Update the texture cache.
    textureCache.Add(&srcTexture, 1);
    float textureUnit = static_cast<float>(textureCache.GetTextureUnit(srcTexture));

    float maskTextureUnit;

    if (maskTexture == 0)
    {
        // Don't use a mask.
        maskTextureUnit = 0;
        maskX = 0;
        maskY = 0;
        maskW = 0;
        maskH = 0;
    }
    else
    {
        textureCache.Add(&maskTexture, 1);
        maskTextureUnit = static_cast<float>(textureCache.GetTextureUnit(maskTexture));
    }

    float paletteTextureUnit;

    if (paletteTexture == 0)
    {
        // Don't use a palette.
        paletteTextureUnit = 0;
        paletteX = 0;
        paletteY = 0;
        paletteW = 0;
    }
    else
    {
        textureCache.Add(&paletteTexture, 1);
        paletteTextureUnit = static_cast<float>(textureCache.GetTextureUnit(paletteTexture));
    }

    // Add the data to the generalRenderData.
    generalRenderData.push_back(destX);
    generalRenderData.push_back(destY);
    generalRenderData.push_back(textureUnit);
    generalRenderData.push_back(srcX);
    generalRenderData.push_back(srcY);
    generalRenderData.push_back(0);
    generalRenderData.push_back(alpha);
    generalRenderData.push_back(maskTextureUnit);
    generalRenderData.push_back(maskX);
    generalRenderData.push_back(maskY);
    generalRenderData.push_back(paletteTextureUnit);
    generalRenderData.push_back(paletteX);
    generalRenderData.push_back(paletteY);
    generalRenderData.push_back(paletteW);

    for (long i=0; i<2; i++)
    {
        generalRenderData.push_back(destX + destW);
        generalRenderData.push_back(destY);
        generalRenderData.push_back(textureUnit);
        generalRenderData.push_back(srcX + srcW);
        generalRenderData.push_back(srcY);
        generalRenderData.push_back(0);
        generalRenderData.push_back(alpha);
        generalRenderData.push_back(maskTextureUnit);
        generalRenderData.push_back(maskX + maskW);
        generalRenderData.push_back(maskY);
        generalRenderData.push_back(paletteTextureUnit);
        generalRenderData.push_back(paletteX);
        generalRenderData.push_back(paletteY);
        generalRenderData.push_back(paletteW);

        generalRenderData.push_back(destX);
        generalRenderData.push_back(destY + destH);
        generalRenderData.push_back(textureUnit);
        generalRenderData.push_back(srcX);
        generalRenderData.push_back(srcY + srcH);
        generalRenderData.push_back(0);
        generalRenderData.push_back(alpha);
        generalRenderData.push_back(maskTextureUnit);
        generalRenderData.push_back(maskX);
        generalRenderData.push_back(maskY + maskH);
        generalRenderData.push_back(paletteTextureUnit);
        generalRenderData.push_back(paletteX);
        generalRenderData.push_back(paletteY);
        generalRenderData.push_back(paletteW);
    }

    generalRenderData.push_back(destX + destW);
    generalRenderData.push_back(destY + destH);
    generalRenderData.push_back(textureUnit);
    generalRenderData.push_back(srcX + srcW);
    generalRenderData.push_back(srcY + srcH);
    generalRenderData.push_back(0);
    generalRenderData.push_back(alpha);
    generalRenderData.push_back(maskTextureUnit);
    generalRenderData.push_back(maskX + maskW);
    generalRenderData.push_back(maskY + maskH);
    generalRenderData.push_back(paletteTextureUnit);
    generalRenderData.push_back(paletteX);
    generalRenderData.push_back(paletteY);
    generalRenderData.push_back(paletteW);

    return;
}

void Graphics::DrawTriangle(uint32_t destTexture,
                            float x1, float y1, const Color& c1,
                            float x2, float y2, const Color& c2,
                            float x3, float y3, const Color& c3)
{
    // Update the drawFramebuffer if the destination texture has changed.
    if (destTexture != framebuffers[drawFramebuffer])
    {
        FlushRenderData();
        UseFramebuffer(drawFramebuffer);
        AttachTexture(drawFramebuffer, destTexture);
    }

    // Add the data to the generalRenderData.
    generalRenderData.push_back(x1);
    generalRenderData.push_back(y1);
    generalRenderData.push_back(0);
    generalRenderData.push_back(c1.r);
    generalRenderData.push_back(c1.g);
    generalRenderData.push_back(c1.b);
    generalRenderData.push_back(c1.a);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);

    generalRenderData.push_back(x2);
    generalRenderData.push_back(y2);
    generalRenderData.push_back(0);
    generalRenderData.push_back(c2.r);
    generalRenderData.push_back(c2.g);
    generalRenderData.push_back(c2.b);
    generalRenderData.push_back(c2.a);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);

    generalRenderData.push_back(x3);
    generalRenderData.push_back(y3);
    generalRenderData.push_back(0);
    generalRenderData.push_back(c3.r);
    generalRenderData.push_back(c3.g);
    generalRenderData.push_back(c3.b);
    generalRenderData.push_back(c3.a);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);
    generalRenderData.push_back(0);

    return;
}

void Graphics::UpdateTextureCache(const uint32_t* textures, size_t size)
{
    textureCache.Add(textures, size);
    return;
}

void Graphics::ClearTextureCache()
{
    textureCache.RemoveAll();
    return;
}

void Graphics::FlushRenderData()
{
    if (generalRenderData.size() == 0)
        return; // Nothing to render.

    // Set the target output.
    uint32_t textureOutput = framebuffers[drawFramebuffer];
    if (!textureOutput)
    {
        // There is no output texture.
        generalRenderData.resize(0);
        return;
    }

    UseFramebuffer(drawFramebuffer);

    // Set the method and parameters used to draw.
    const TextureData& texData = textures[textureOutput];

    UseShaderProgram(generalRenderShaderProgram);
    SetUniform(generalRenderShaderProgram, "outputWidth",  texData.width);
    SetUniform(generalRenderShaderProgram, "outputHeight", texData.height);

    SetViewport(texData.width, texData.height);

    // Set the input.
    UseVertexArray(generalRenderVertexArray);

    // Copy the vertex data to the render buffer.
    UseArrayBuffer(generalRenderBuffer);
    SetArrayData(generalRenderBuffer,
                 generalRenderData.data(),
                 generalRenderData.size()*sizeof(float));

    // Execute the command to draw.
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(generalRenderData.size()/10));
    CheckGlErrors("Drawing on image");

    // Clear the queue of data waiting to be drawn.
    generalRenderData.resize(0);
    return;
}

} // End of namespace mi.
