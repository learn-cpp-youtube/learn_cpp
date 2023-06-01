#include "ShaderLibrary.h"

namespace mi
{
namespace shader
{
namespace vertex
{
const std::string blitToScreen =
"#version 330                          \n"
"                                        "
"layout (location = 0) in vec2 vPos;     "
"layout (location = 1) in vec2 tPos;     "
"out vec2 texPos;                        "
"                                        "
"void main()                             "
"{                                       "
"    gl_Position = vec4(2.0*vPos.x - 1.0,"
"                       1.0 - 2.0*vPos.y,"
"                       0.5,             "
"                       1.0);            "
"    texPos = tPos;                      "
"}                                       ";

const std::string blitToImage =
"#version 330                          \n"
"                                        "
"layout (location = 0) in vec2 vPos;     "
"layout (location = 1) in vec2 tPos;     "
"out vec2 texPos;                        "
"                                        "
"void main()                             "
"{                                       "
"    gl_Position = vec4(2.0*vPos.x - 1.0,"
"                       2.0*vPos.y - 1.0,"
"                       0.5,             "
"                       1.0);            "
"    texPos = tPos;                      "
"}                                       ";

const std::string drawToImage =
"#version 330                                              \n"
"                                                            "
"layout (location = 0) in vec2  pPos;                        "
"layout (location = 1) in float pType;                       "
"layout (location = 2) in vec4  pCol;                        "
"layout (location = 3) in float pMaskType;                   "
"layout (location = 4) in vec2  pMaskPos;                    "
"layout (location = 5) in float pPaletteType;                "
"layout (location = 6) in vec2  pPalettePos;                 "
"layout (location = 7) in float pPaletteWidth;               "
"                                                            "
"uniform int outputWidth;                                    "
"uniform int outputHeight;                                   "
"                                                            "
"uniform sampler2D tex1;                                     "
"uniform sampler2D tex2;                                     "
"uniform sampler2D tex3;                                     "
"uniform sampler2D tex4;                                     "
"uniform sampler2D tex5;                                     "
"uniform sampler2D tex6;                                     "
"uniform sampler2D tex7;                                     "
"uniform sampler2D tex8;                                     "
"                                                            "
"flat out float vType;                                       "
"     out vec4  vCol;                                        "
"flat out float vMaskType;                                   "
"     out vec2  vMaskPos;                                    "
"flat out float vPaletteType;                                "
"flat out vec2  vPalettePos;                                 "
"flat out float vPaletteWidth;                               "
"flat out vec2  vPaletteTexDim;                              "
"                                                            "
"void main()                                                 "
"{                                                           "
"    gl_Position = vec4(2.0*(pPos.x/float(outputWidth))-1.0, "
"                       2.0*(pPos.y/float(outputHeight))-1.0,"
"                       0.5,                                 "
"                       1);                                  "
"                                                            "
"    vType     = pType;                                      "
"    vMaskType = pMaskType;                                  "
"                                                            "
"    int   type;                                             "
"    ivec2 dim;                                              "
"                                                            "
"    type = int(pType+0.5);                                  "
"                                                            "
"    dim = int(type==0) * ivec2(255, 255)                    "
"        + int(type==1) * textureSize(tex1, 0)               "
"        + int(type==2) * textureSize(tex2, 0)               "
"        + int(type==3) * textureSize(tex3, 0)               "
"        + int(type==4) * textureSize(tex4, 0)               "
"        + int(type==5) * textureSize(tex5, 0)               "
"        + int(type==6) * textureSize(tex6, 0)               "
"        + int(type==7) * textureSize(tex7, 0)               "
"        + int(type==8) * textureSize(tex8, 0);              "
"                                                            "
"    vCol.r = pCol.r/float(dim.x);                           "
"    vCol.g = pCol.g/float(dim.y);                           "
"    vCol.b = pCol.b/255.0;                                  "
"    vCol.a = pCol.a/255.0;                                  "
"                                                            "
"    type = int(pMaskType+0.5);                              "
"                                                            "
"    dim = int(type==0) * ivec2(1, 1)                        "
"        + int(type==1) * textureSize(tex1, 0)               "
"        + int(type==2) * textureSize(tex2, 0)               "
"        + int(type==3) * textureSize(tex3, 0)               "
"        + int(type==4) * textureSize(tex4, 0)               "
"        + int(type==5) * textureSize(tex5, 0)               "
"        + int(type==6) * textureSize(tex6, 0)               "
"        + int(type==7) * textureSize(tex7, 0)               "
"        + int(type==8) * textureSize(tex8, 0);              "
"                                                            "
"    vMaskPos.x = pMaskPos.x/float(dim.x);                   "
"    vMaskPos.y = pMaskPos.y/float(dim.y);                   "
"                                                            "
"    vPaletteType  = pPaletteType;                           "
"    vPalettePos   = pPalettePos;                            "
"                                                            "
"    type = int(pPaletteType+0.5);                           "
"                                                            "
"    dim = int(type==0) * ivec2(1, 1)                        "
"        + int(type==1) * textureSize(tex1, 0)               "
"        + int(type==2) * textureSize(tex2, 0)               "
"        + int(type==3) * textureSize(tex3, 0)               "
"        + int(type==4) * textureSize(tex4, 0)               "
"        + int(type==5) * textureSize(tex5, 0)               "
"        + int(type==6) * textureSize(tex6, 0)               "
"        + int(type==7) * textureSize(tex7, 0)               "
"        + int(type==8) * textureSize(tex8, 0);              "
"                                                            "
"    vPaletteTexDim = vec2(dim);                             "
"                                                            "
"    int dontDefault = int(type!=0)                          "
"                    * int(int(pPaletteWidth+0.5)>0);        "
"    vPaletteWidth = float(dontDefault)*pPaletteWidth        "
"                  + float(1-dontDefault)*1.0;               "
"}                                                           ";

} // End of namespace mi::shader::vertex.

namespace fragment
{
const std::string blitToScreen =
"#version 330                       \n"
"                                     "
"in  vec2 texPos;                     "
"out vec4 fragColor;                  "
"                                     "
"uniform sampler2D tex;               "
"                                     "
"void main()                          "
"{                                    "
"    fragColor = texture(tex, texPos);"
"}                                    ";

const std::string createMask =
"#version 330                                \n"
"                                              "
"in  vec2 texPos;                              "
"out vec4 fragColor;                           "
"                                              "
"uniform sampler2D tex;                        "
"uniform ivec4 colorKey;                       "
"uniform ivec4 matchColor;                     "
"uniform ivec4 noMatchColor;                   "
"                                              "
"void main()                                   "
"{                                             "
"    vec4 c = texture(tex, texPos);            "
"    int isMatch =                             "
"          int(int(c.r*255.0+0.5)==colorKey.r) "
"        * int(int(c.g*255.0+0.5)==colorKey.g) "
"        * int(int(c.b*255.0+0.5)==colorKey.b) "
"        * int(int(c.a*255.0+0.5)==colorKey.a);"
"                                              "
"    ivec4 m = isMatch*matchColor              "
"             +(1-isMatch)*noMatchColor;       "
"                                              "
"    fragColor = vec4(float(m.r)/255.0,        "
"                     float(m.g)/255.0,        "
"                     float(m.b)/255.0,        "
"                     float(m.a)/255.0);       "
"}                                             ";

const std::string addAlpha =
"#version 330                         \n"
"                                       "
"in  vec2 texPos;                       "
"out vec4 fragColor;                    "
"                                       "
"uniform sampler2D texColor;            "
"uniform sampler2D texAlpha;            "
"                                       "
"void main()                            "
"{                                      "
"    vec4 c = texture(texColor, texPos);"
"    vec4 a = texture(texAlpha, texPos);"
"                                       "
"    fragColor = vec4(c.rgb*a.r, a.r);  "
"}                                      ";

const std::string drawToImage =
"#version 330                                             \n"
"                                                           "
"flat in float vType;                                       "
"     in vec4  vCol;                                        "
"flat in float vMaskType;                                   "
"     in vec2  vMaskPos;                                    "
"flat in float vPaletteType;                                "
"flat in vec2  vPalettePos;                                 "
"flat in float vPaletteWidth;                               "
"flat in vec2  vPaletteTexDim;                              "
"                                                           "
"uniform sampler2D tex1;                                    "
"uniform sampler2D tex2;                                    "
"uniform sampler2D tex3;                                    "
"uniform sampler2D tex4;                                    "
"uniform sampler2D tex5;                                    "
"uniform sampler2D tex6;                                    "
"uniform sampler2D tex7;                                    "
"uniform sampler2D tex8;                                    "
"                                                           "
"out vec4 fragColor;                                        "
"                                                           "
"void main()                                                "
"{                                                          "
"    int type = int(vType+0.5);                             "
"                                                           "
"    vec4 color = float(type==0) * vec4(vCol.rgb, 1)        "
"               + float(type==1) * texture(tex1, vCol.xy)   "
"               + float(type==2) * texture(tex2, vCol.xy)   "
"               + float(type==3) * texture(tex3, vCol.xy)   "
"               + float(type==4) * texture(tex4, vCol.xy)   "
"               + float(type==5) * texture(tex5, vCol.xy)   "
"               + float(type==6) * texture(tex6, vCol.xy)   "
"               + float(type==7) * texture(tex7, vCol.xy)   "
"               + float(type==8) * texture(tex8, vCol.xy);  "
"                                                           "
"    type = int(vPaletteType+0.5);                          "
"                                                           "
"    int paletteIndex = int(color.r*255.0+0.5)*256*256      "
"                     + int(color.g*255.0+0.5)*256          "
"                     + int(color.b*255.0+0.5);             "
"                                                           "
"    int  paletteWidth = int(vPaletteWidth+0.5);            "
"    vec2 palettePos;                                       "
"    palettePos.x = (vPalettePos.x + 0.5                    "
"                   + float(paletteIndex % paletteWidth))   "
"                   / vPaletteTexDim.x;                     "
"    palettePos.y = (vPalettePos.y + 0.5                    "
"                   + float(paletteIndex / paletteWidth))   "
"                   / vPaletteTexDim.y;                     "
"                                                           "
"    fragColor = float(type==0) * color                     "
"              + float(type==1) * texture(tex1, palettePos) "
"              + float(type==2) * texture(tex2, palettePos) "
"              + float(type==3) * texture(tex3, palettePos) "
"              + float(type==4) * texture(tex4, palettePos) "
"              + float(type==5) * texture(tex5, palettePos) "
"              + float(type==6) * texture(tex6, palettePos) "
"              + float(type==7) * texture(tex7, palettePos) "
"              + float(type==8) * texture(tex8, palettePos);"
"                                                           "
"    float maskAlpha;                                       "
"    type = int(vMaskType+0.5);                             "
"                                                           "
"    maskAlpha = float(type==0) * 1.0                       "
"              + float(type==1) * texture(tex1, vMaskPos).r "
"              + float(type==2) * texture(tex2, vMaskPos).r "
"              + float(type==3) * texture(tex3, vMaskPos).r "
"              + float(type==4) * texture(tex4, vMaskPos).r "
"              + float(type==5) * texture(tex5, vMaskPos).r "
"              + float(type==6) * texture(tex6, vMaskPos).r "
"              + float(type==7) * texture(tex7, vMaskPos).r "
"              + float(type==8) * texture(tex8, vMaskPos).r;"
"                                                           "
"    fragColor *= vCol.a * maskAlpha;                       "
"}                                                          ";

} // End of namespace mi::shader::fragment.

} // End of namespace mi::shader.

} // End of namespace mi.