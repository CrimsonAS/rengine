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
    GlyphTextureJob(GlyphContext *context, const std::string &text, int pixelSize)
        : m_context(context)
        , m_text(text)
        , m_pixelSize(pixelSize)
    {
    }

    void onExecute() override;

private:
    GlyphContext *m_context;
    std::string m_text;
    int m_pixelSize;
};


GlyphContext::GlyphContext(const std::string &fontFile)
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

GlyphContext::~GlyphContext()
{
    free(m_fontData);
}

void GlyphTextureJob::onExecute()
{
    const stbtt_fontinfo *fontInfo = m_context->fontInfo();
    float scale = stbtt_ScaleForPixelHeight(fontInfo, m_pixelSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(fontInfo, &ascent, &descent, &lineGap);

    std::cout << "GlyphTextureJob::onExecute: text='" << m_text
              << "', size=" << m_pixelSize
              << ", scale=" << scale
              << ", ascent=" << ascent
              << ", descent=" << descent
              << ", lineGap=" << lineGap
              << std::endl;

    std::vector<int> glyphs;

    float width = 0;

    for (int i=0; m_text[i]; ++i) {
        int g = stbtt_FindGlyphIndex(fontInfo, m_text[i]);
        if (i > 1) {
            width += scale*stbtt_GetCodepointKernAdvance(fontInfo, glyphs.back(), g);
        }
        int advance, leftSideBearing;
        stbtt_GetGlyphHMetrics(fontInfo, g, &advance, &leftSideBearing);
        std::cout << " - '" << m_text[i] << "': index=" << g << ", advance=" << advance << ", lsb=" << leftSideBearing << std::endl;
        width += scale * advance;

        glyphs.push_back(g);
    }

    std::cout << " - total width is: " << std::ceil(width) << std::endl;
}

RENGINE_END_NAMESPACE

