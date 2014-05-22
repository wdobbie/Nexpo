#include "font.h"
#include <fstream>
#include <string>
#include <iostream>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u) malloc(x)
#define STBTT_free(x,u) free(x)
#include "stb_truetype.h"

class Font
{
public:
    Font();
    ~Font();

    stbtt_fontinfo m_fontInfo;
    std::string m_fontData;
    float m_scale;

};

Font::Font()
{
}

Font::~Font()
{

}

LUAEXPORT(Font* loadFont(const char *path))
{
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) {
        return 0;
    }

    Font* font = new Font;

    in.seekg(0, std::ios::end);
    font->m_fontData.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&font->m_fontData[0], font->m_fontData.size());
    in.close();

    const unsigned char* data = (const unsigned char*)font->m_fontData.data();
    int index = stbtt_GetFontOffsetForIndex(data, 0);
    if (stbtt_InitFont(&font->m_fontInfo, data, index) == 0) {
        delete font;
        return 0;
    }

    // Scale all coordinates so font is roughly two units tall
    // This corresponds to a centred character spanning from -1 to 1
    font->m_scale = stbtt_ScaleForPixelHeight(&font->m_fontInfo, 2.0f);

    return font;
}

LUAEXPORT(Path* pathForCodepoint(Font* font, int codepoint))
{
    if (!font) {
        std::cerr << "Error: font is null" << std::endl;
    }

    stbtt_vertex* verts;
    int nvert = stbtt_GetCodepointShape(&font->m_fontInfo, codepoint, &verts);
    if (nvert == 0) {
        return 0;
    }

    int x0, y0, x1, y1;
    stbtt_GetCodepointBox(&font->m_fontInfo, codepoint, &x0, &y0, &x1, &y1);
    int xoffset = -(x1 + x0)/2;
    int yoffset = -(y1 + y0)/2;

    Path* path = newPath();

    for (int i=0; i<nvert; i++) {
        stbtt_vertex& v = verts[i];
        switch(v.type) {
        case STBTT_vmove:
            moveTo(path, (v.x+xoffset) * font->m_scale, (v.y+yoffset) * font->m_scale);
            break;
        case STBTT_vline:
            lineTo(path, (v.x+xoffset) * font->m_scale, (v.y+yoffset) * font->m_scale);
            break;
        case STBTT_vcurve:
            quadraticCurveTo(path,
                             (v.cx+xoffset) * font->m_scale,
                             (v.cy+yoffset) * font->m_scale,
                             (v.x+xoffset) * font->m_scale,
                             (v.y+yoffset) * font->m_scale);

            break;
        default:
            std::cerr << "Unknown vertex type in font: " << v.type << " -- ";
        }
    }

    return path;
}

LUAEXPORT(void freeFont(Font* font))
{
    delete font;
}
