#pragma once

#include "stb_truetype.h"

#include <iostream>
#include <fstream>

RENGINE_BEGIN_NAMESPACE



class GlyphContext
{
public:
    GlyphContext(const std::string &fontFile);
    ~GlyphContext();

    bool isValid() const { return m_fontData; }

    const stbtt_fontinfo *fontInfo() const { return &m_fontInfo; }

private:
    unsigned char *m_fontData = nullptr;
    stbtt_fontinfo m_fontInfo;
};



class GlyphTextureJob : public WorkQueue::Job
{
public:
    GlyphTextureJob(GlyphContext *context, const std::string &text, int pixelSize, vec4 color = vec4(1, 1, 1, 1))
        : m_context(context)
        , m_text(text)
        , m_color(color)
        , m_pixelSize(pixelSize)
    {
    }

    void onExecute() override;

    vec2 textureSize() const { return vec2(m_textureWidth, m_textureHeight); }
    unsigned int *textureData() const { return m_textureData.get(); }

private:
    void renderSingleGlyph(int x, int y,                    // the position, x, y
                       unsigned int *t, int tw, int th,     // the target texture data, 32-bit RGBA
                       unsigned char *b, int bw, int bh,    // the source glyph bitmap, 8 bit alpha mask
                       int cr, int cg, int cb, int ca);     // the glyph color..

    GlyphContext *m_context;
    std::string m_text;
    vec4 m_color;
    int m_pixelSize;
    int m_textureWidth = 0;
    int m_textureHeight = 0;
    std::shared_ptr<unsigned int> m_textureData;
};



inline GlyphContext::GlyphContext(const std::string &fontFile)
{
    std::ifstream file(fontFile, std::ios::binary | std::ios::in | std::ios::ate);
    if (!file.is_open()) {
        loge << "Failed to open font file '" << fontFile << "'" << std::endl;
        return;
    }

    int size = file.tellg();

    m_fontData = (unsigned char *) malloc(size);
    if (!m_fontData) {
        loge << "Failed to allocate font data for '" << fontFile << "'" << std::endl;
        return;
    }

    file.seekg(0, std::ios::beg);
    file.read((char *) m_fontData, size);
    file.close();

    if (!stbtt_InitFont(&m_fontInfo, m_fontData, 0)) {
        loge << "Failed to initialize font '" << fontFile << "'" << std::endl;
        free(m_fontData);
        m_fontData = 0;
        return;
    }
}

inline GlyphContext::~GlyphContext()
{
    free(m_fontData);
}

inline void GlyphTextureJob::onExecute()
{
    auto start = std::chrono::system_clock::now();

    const stbtt_fontinfo *fontInfo = m_context->fontInfo();
    float scale = stbtt_ScaleForPixelHeight(fontInfo, m_pixelSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(fontInfo, &ascent, &descent, &lineGap);

    // std::cout << "GlyphTextureJob::onExecute: text='" << m_text
    //           << "', size=" << m_pixelSize
    //           << ", scale=" << scale
    //           << ", ascent=" << ascent
    //           << ", descent=" << descent
    //           << ", lineGap=" << lineGap
    //           << std::endl;

    std::vector<int> glyphs;
    std::vector<float> advances;

    float xpos = 0;

    int maxBmWidth = 0;
    int maxBmHeight = 0;

    for (int i=0; m_text[i]; ++i) {
        int g = stbtt_FindGlyphIndex(fontInfo, m_text[i]);
        if (i > 1) {
            xpos += scale*stbtt_GetCodepointKernAdvance(fontInfo, glyphs.back(), g);
        }
        int advance, leftSideBearing;
        stbtt_GetGlyphHMetrics(fontInfo, g, &advance, &leftSideBearing);

        float xShift = xpos - (float) std::floor(xpos);
        int x0, x1, y0, y1;
        stbtt_GetGlyphBitmapBoxSubpixel(fontInfo, g, scale, scale, xShift, 0, &x0, &y0, &x1, &y1);
        maxBmWidth = std::max(x1 - x0, maxBmWidth);
        maxBmHeight = std::max(y1 - y0, maxBmHeight);

        // std::cout << " - '" << m_text[i] << "': index=" << g << ", advance=" << advance << "/" << xpos << ", lsb=" << leftSideBearing
        //           << ", box=" << x0 << "-" << x1 << ", " << y0 << "-" << y1 << std::endl;

        xpos += scale * (advance - leftSideBearing);

        advances.push_back(xpos);
        glyphs.push_back(g);
    }

    m_textureWidth = std::ceil(xpos);
    m_textureHeight = std::ceil((ascent - descent) * scale);

    // std::cout << " - dimensions texture=" << m_textureWidth << "x" << m_textureHeight
    //           << ", bitmap=" << maxBmWidth << "x" << maxBmHeight << std::endl;

    unsigned char *bmData = (unsigned char *) malloc(maxBmWidth * maxBmHeight);
    unsigned int *textureData = (unsigned int *) calloc(m_textureWidth * m_textureHeight, 4);
    m_textureData = std::shared_ptr<unsigned int>(textureData, free);

    int ca = m_color.w * 255;
    int cr = m_color.w * m_color.x * 255;
    int cg = m_color.w * m_color.y * 255;
    int cb = m_color.w * m_color.z * 255;

    float x = 0;
    float y = ascent * scale;
    for (int i=0; i<glyphs.size(); ++i) {
        int glyph = glyphs[i];

        int x0, y0, x1, y1;

        float xShift = x - (float) std::floor(x);
        stbtt_GetGlyphBitmapBoxSubpixel(fontInfo, glyph, scale, scale, xShift, 0, &x0, &y0, &x1, &y1);

        assert(x1 - x0 <= maxBmWidth);
        assert(y1 - y0 <= maxBmHeight);

        int bmw = x1 - x0;
        int bmh = y1 - y0;
        stbtt_MakeGlyphBitmapSubpixel(fontInfo, bmData, bmw, bmh, bmw, scale, scale, xShift, 0, glyph);

        renderSingleGlyph(x, y + y0,
                          textureData, m_textureWidth, m_textureHeight,
                          bmData, bmw, bmh,
                          cr, cg, cb, ca);

        x = advances[i];
    }

    free(bmData);

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "generated glyph texture for '" << m_text << "' in " << diff.count() * 1000 << " ms" << std::endl;
}

inline void GlyphTextureJob::renderSingleGlyph(int x, int y,
                                        unsigned int *t, int tw, int th,
                                        unsigned char *b, int bw, int bh,
                                        int cr, int cg, int cb, int ca)
{
    for (int yy=0; yy<bh; ++yy) {
        int dy = yy + y;

        assert(dy >= 0);
        assert(dy < th);
        // if (yy + y < 0)
        //     continue;
        // else if (yy + y > ctx->buffer->height)
        //     break;
        unsigned char *src = b + yy * bw;
        unsigned int *dst = t + dy * tw + x;

        for (int xx=0; xx<bw; ++xx) {
            assert(x + xx >= 0);
            assert(x + xx < tw);
            unsigned char s = src[xx];
            if (s == 0)
                continue;
            // if (xx + x < 0)
            //     continue;
            // else if (xx + x > ctx->buffer->width)
            //     break;
            int alpha = ca * s / 255;

            int r, g, b;
            if (alpha == 255) {
                r = cr;
                g = cg;
                b = cb;
            } else {
                r = (alpha * cr) >> 8;
                g = (alpha * cg) >> 8;
                b = (alpha * cb) >> 8;
            }

            dst[xx] = (alpha << 24)
                      | (b << 16)
                      | (g << 8)
                      | r;
        }
    }
    // color[3] = alpha;

}


RENGINE_END_NAMESPACE

