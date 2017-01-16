/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <stack>
#include <stdio.h>
#include <alloca.h>
#include <iomanip>
#include <cstring>

#include "openglrenderer_shaders.h"

RENGINE_BEGIN_NAMESPACE

class OpenGLRenderer : public Renderer
{
public:

    struct TexturePool : public std::vector<GLuint>
    {
        ~TexturePool()
        {
            glDeleteTextures(size(), data());
        }

        GLuint acquire() {
            GLuint id;
            if (empty()) {
                glGenTextures(1, &id);
                return id;
            }
            id = back();
            pop_back();
            return id;
        }

        void release(GLuint id) {
            assert(id > 0);
            push_back(id);
        }

        void compact() {
            glFlush();
            for (auto id : *this) {
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }
        }
    };

    struct Element {
        Node *node;
        unsigned vboOffset;         // offset into vbo for flattened, rect and layer nodes
        float z;                    // only valid when 'projection' is set
        unsigned texture;           // only valid during rendering when 'layered' is set.
        unsigned sourceTexture;     // only valid during rendering when 'layered' is set and we have a shadow node
        unsigned groupSize : 29;    // The size of this group, used with 'projection' and 'layered'. Packed to ft into 32-bit
                                    // The groupSize is the number of nodes inside the group, excluding the parent.
        unsigned projection : 1;    // 3d subtree
        unsigned layered : 1;       // subtree is flattened into a layer (texture)
        unsigned completed : 1;     // used during the actual rendering to know we're done with it

        bool operator<(const Element &e) const { return e.completed || z < e.z; }
    };
    struct Program : OpenGLShaderProgram {
        int matrix;
    };
    enum ProgramUpdate {
        UpdateSolidProgram          = 0x01,
        UpdateTextureProgram        = 0x02,
        UpdateTextureBgrProgram     = 0x04,
        UpdateAlphaTextureProgram   = 0x08,
        UpdateColorFilterProgram    = 0x10,
        UpdateBlurProgram           = 0x20,
        UpdateShadowProgram         = 0x40,
        UpdateAllPrograms           = 0xffffffff
    };

    OpenGLRenderer();
    ~OpenGLRenderer();

    Texture *createTextureFromImageData(vec2 size, Texture::Format format, void *data) override;

    void initialize() override;
    bool render() override;
    void frameSwapped() override { m_texturePool.compact(); }
    bool readPixels(int x, int y, int w, int h, unsigned *pixels) override;

    void prepass(Node *n);
    void build(Node *n);
    void drawColorQuad(unsigned bufferOffset, vec4 color);
    void drawTextureQuad(unsigned bufferOffset, GLuint texId, float opacity = 1.0, Texture::Format format = Texture::RGBA_32);
    void drawColorFilterQuad(unsigned bufferOffset, GLuint texId, mat4 cm);
    void drawBlurQuad(unsigned bufferOffset, GLuint texId, int radius, vec2 renderSize, vec2 textureSize, vec2 step);
    void drawShadowQuad(unsigned bufferOffset, GLuint texId, int radius, vec2 renderSize, vec2 textureSize, vec2 step, vec4 color);
    void activateShader(const Program *shader);
    void projectQuad(vec2 a, vec2 b, vec2 *v);
    void render(Element *first, Element *last);
    void renderToLayer(Element *e);
    void setDefaultOpenGLState();
    rect2d boundingRectFor(unsigned vertexOffset) const { return rect2d(m_vertices[vertexOffset], m_vertices[vertexOffset + 3]); }

    void ensureMatrixUpdated(ProgramUpdate bit, Program *p);

    Program prog_texture;
    Program prog_texture_bgr;
    struct : public Program {
        int alpha;
    } prog_alphaTexture;
    struct : public Program {
        int color;
    } prog_solid;
    struct : public Program {
        int colorMatrix;
    } prog_colorFilter;
    struct BlurProgram : public Program {
        int dims;
        int radius;
        int sigma;
        int step;
    } prog_blur;
    struct : public BlurProgram {
        int color;
    } prog_shadow;

    unsigned m_numLayeredNodes;
    unsigned m_numTextureNodes;
    unsigned m_numRectangleNodes;
    unsigned m_numTransformNodes;
    unsigned m_numTransformNodesWith3d;
    unsigned m_numRenderNodes;
    unsigned m_additionalQuads;

    unsigned m_vertexIndex;
    unsigned m_elementIndex;
    vec2 *m_vertices;
    Element *m_elements;
    mat4 m_proj;
    mat4 m_m2d;    // for the 2d world
    mat4 m_m3d;    // below a 3d projection subtree
    float m_farPlane;
    rect2d m_layerBoundingBox;
    vec2 m_surfaceSize;

    TexturePool m_texturePool;

    const Program *m_activeShader;
    GLuint m_texCoordBuffer;
    GLuint m_vertexBuffer;
    GLuint m_fbo;

    unsigned m_matrixState;

    bool m_render3d : 1;
    bool m_layered : 1;
    bool m_srgb : 1;

};

inline void OpenGLRenderer::projectQuad(vec2 a, vec2 b, vec2 *v)
{
    // The steps involved in each line is as follows.:
    // pt_3d = matrix3D * pt                 // apply the 3D transform
    // pt_proj = pt_3d.project2D()           // project it to 2D based on current farPlane
    // pt_screen = parent_matrix * pt_proj   // Put the output of our local 3D into the scene world coordinate system
    v[0] = m_m2d * ((m_m3d * vec3(a))       .project2D(m_farPlane));    // top left
    v[1] = m_m2d * ((m_m3d * vec3(a.x, b.y)).project2D(m_farPlane));    // bottom left
    v[2] = m_m2d * ((m_m3d * vec3(b.x, a.y)).project2D(m_farPlane));    // top right
    v[3] = m_m2d * ((m_m3d * vec3(b))       .project2D(m_farPlane));    // bottom right
}

inline void OpenGLRenderer::ensureMatrixUpdated(ProgramUpdate bit, Program *p)
{
    if (m_matrixState & bit) {
        m_matrixState &= ~bit;
        glUniformMatrix4fv(p->matrix, 1, true, m_proj.m);
    }
}

inline OpenGLRenderer::OpenGLRenderer()
    : m_numLayeredNodes(0)
    , m_numTextureNodes(0)
    , m_numRectangleNodes(0)
    , m_numTransformNodes(0)
    , m_numTransformNodesWith3d(0)
    , m_additionalQuads(0)
    , m_vertexIndex(0)
    , m_elementIndex(0)
    , m_vertices(0)
    , m_elements(0)
    , m_farPlane(0)
    , m_activeShader(0)
    , m_texCoordBuffer(0)
    , m_vertexBuffer(0)
    , m_fbo(0)
    , m_matrixState(UpdateAllPrograms)
    , m_render3d(false)
    , m_layered(false)
    , m_srgb(false)
{
    initialize();
}

inline OpenGLRenderer::~OpenGLRenderer()
{
    glDeleteBuffers(1, &m_texCoordBuffer);
    glDeleteBuffers(1, &m_vertexBuffer);

    assert(m_fbo == 0);
}

inline bool OpenGLRenderer::readPixels(int x, int y, int w, int h, unsigned *bytes)
{
    // Read line-by-line and flip it so we get what we want out..
    // Slow as hell, but this is used for autotesting, so who cares..
    for (int i=0; i<h; ++i)
        glReadPixels(x, y + h - i - 1, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, bytes + i * w);
    return true;
}

inline Texture *OpenGLRenderer::createTextureFromImageData(vec2 size, Texture::Format format, void *data)
{
    OpenGLTexture *texture = new OpenGLTexture();
    texture->setFormat(format);
    texture->upload(size.x, size.y, data);
    return texture;
}

inline void OpenGLRenderer::initialize()
{
    {   // Create a texture coordinate buffer
        const float data[] = { 0, 0, 0, 1, 1, 0, 1, 1 };
        glGenBuffers(1, &m_texCoordBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Create the vertex coordinate buffer
    glGenBuffers(1, &m_vertexBuffer);

    std::vector<const char *> attrsVT;
    attrsVT.push_back("aV");
    attrsVT.push_back("aT");

    std::vector<const char *> attrsV;
    attrsV.push_back("aV");

    // Default texture shader
    prog_texture.initialize(openglrenderer_vsh_texture(), openglrenderer_fsh_texture(), attrsVT);
    prog_texture.matrix = prog_texture.resolve("m");

    // BGRA texture shader
    prog_texture_bgr.initialize(openglrenderer_vsh_texture(), openglrenderer_fsh_texture_bgra(), attrsVT);
    prog_texture_bgr.matrix = prog_texture.resolve("m");

    // Alpha texture shader
    prog_alphaTexture.initialize(openglrenderer_vsh_texture(), openglrenderer_fsh_texture_alpha(), attrsVT);
    prog_alphaTexture.matrix = prog_alphaTexture.resolve("m");
    prog_alphaTexture.alpha = prog_alphaTexture.resolve("alpha");

    // Solid color shader...
    prog_solid.initialize(openglrenderer_vsh_solid(), openglrenderer_fsh_solid(), attrsV);
    prog_solid.matrix = prog_solid.resolve("m");
    prog_solid.color = prog_solid.resolve("color");

    // Color filter shader..
    prog_colorFilter.initialize(openglrenderer_vsh_texture(), openglrenderer_fsh_texture_colorfilter(), attrsVT);
    prog_colorFilter.matrix = prog_solid.resolve("m");
    prog_colorFilter.colorMatrix = prog_colorFilter.resolve("CM");

    // Blur shader
    prog_blur.initialize(openglrenderer_vsh_blur(), openglrenderer_fsh_blur(), attrsVT);
    prog_blur.matrix = prog_blur.resolve("m");
    prog_blur.dims = prog_blur.resolve("dims");
    prog_blur.radius = prog_blur.resolve("radius");
    prog_blur.sigma = prog_blur.resolve("sigma");
    prog_blur.step = prog_blur.resolve("step");

    // Shadow shader
    prog_shadow.initialize(openglrenderer_vsh_blur(), openglrenderer_fsh_shadow(), attrsVT);
    prog_shadow.matrix = prog_shadow.resolve("m");
    prog_shadow.dims = prog_shadow.resolve("dims");
    prog_shadow.radius = prog_shadow.resolve("radius");
    prog_shadow.sigma = prog_shadow.resolve("sigma");
    prog_shadow.step = prog_shadow.resolve("step");
    prog_shadow.color = prog_shadow.resolve("color");

    // Using srgb for everything needs a bit more thought as it results in
    // really washed out colors for rectangles and image textures.
    const char *extensions = (const char *) glGetString(GL_EXTENSIONS);
    if (std::strstr(extensions, "GL_ARB_framebuffer_sRGB")) {
        m_srgb = true;
        // glEnable(GL_FRAMEBUFFER_SRGB);
    }

#ifdef RENGINE_LOG_INFO
    static bool logged = false;
    if (!logged) {
        logged = true;
        GLint samples, maxTexSize;
        GLint r, g, b, a, d, s;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glGetIntegerv(GL_SAMPLES, &samples);
        glGetIntegerv(GL_RED_BITS, &r);
        glGetIntegerv(GL_GREEN_BITS, &g);
        glGetIntegerv(GL_BLUE_BITS, &b);
        glGetIntegerv(GL_ALPHA_BITS, &a);
        glGetIntegerv(GL_DEPTH_BITS, &d);
        glGetIntegerv(GL_STENCIL_BITS, &s);
        logi << "OpenGL" << std::endl;
        logi << " - Renderer .........: " << glGetString(GL_RENDERER) << std::endl;
        logi << " - Version ..........: " << glGetString(GL_VERSION) << std::endl;
        logi << " - R/G/B/A ..........: " << r << " " << g << " " << b << " " << a << std::endl;
        logi << " - Depth/Stencil ....: " << d << " " << s << std::endl;
        logi << " - Samples ..........: " << samples << std::endl;
        logi << " - Max Texture Size .: " << maxTexSize << std::endl;
        logi << " - SRGB Rendering ...: " << (m_srgb ? "yes" : "no") << std::endl;
        logi << " - Extensions .......: " << glGetString(GL_EXTENSIONS) << std::endl;
    }
#endif

}

/*!

    Draws a quad using the 'solid' program. \a v is a vector of 8 floats,
    composed of four interleaved x/y points. \a c is the color.

 */
inline void OpenGLRenderer::drawColorQuad(unsigned offset, vec4 c)
{
    activateShader(&prog_solid);
    ensureMatrixUpdated(UpdateSolidProgram, &prog_solid);
    glUniform4f(prog_solid.color, c.x * c.w, c.y * c.w, c.z * c.w, c.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

inline void OpenGLRenderer::drawColorFilterQuad(unsigned offset, GLuint texId, mat4 matrix)
{
    activateShader(&prog_colorFilter);
    ensureMatrixUpdated(UpdateColorFilterProgram, &prog_colorFilter);
    glUniformMatrix4fv(prog_colorFilter.colorMatrix, 1, true, matrix.m);
    // std::cout << prog_colorFilter.colorMatrix << matrix;
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

inline void OpenGLRenderer::drawTextureQuad(unsigned offset, GLuint texId, float opacity, Texture::Format format)
{
    if (opacity == 1) {
        if (format == Texture::BGRA_32 || format == Texture::BGRx_32) {
            activateShader(&prog_texture_bgr);
            ensureMatrixUpdated(UpdateTextureBgrProgram, &prog_texture_bgr);
        } else {
            activateShader(&prog_texture);
            ensureMatrixUpdated(UpdateTextureProgram, &prog_texture);
        }
    } else {
        activateShader(&prog_alphaTexture);
        ensureMatrixUpdated(UpdateAlphaTextureProgram, &prog_alphaTexture);
        glUniform1f(prog_alphaTexture.alpha, opacity);
    }

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

inline void OpenGLRenderer::drawBlurQuad(unsigned offset, GLuint texId, int radius, vec2 renderSize, vec2 textureSize, vec2 step)
{
    activateShader(&prog_blur);
    ensureMatrixUpdated(UpdateBlurProgram, &prog_blur);

    glUniform1i(prog_blur.radius, radius);
    glUniform4f(prog_blur.dims, renderSize.x, renderSize.y, textureSize.x, textureSize.y);
    float sigma = 0.3 * radius + 0.8;
    glUniform1f(prog_blur.sigma, sigma * sigma * 2.0);
    glUniform2f(prog_blur.step, step.x, step.y);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

inline void OpenGLRenderer::drawShadowQuad(unsigned offset, GLuint texId, int radius, vec2 renderSize, vec2 textureSize, vec2 step, vec4 color)
{
    activateShader(&prog_shadow);
    ensureMatrixUpdated(UpdateShadowProgram, &prog_shadow);

    glUniform1i(prog_shadow.radius, radius);
    glUniform4f(prog_shadow.dims, renderSize.x, renderSize.y, textureSize.x, textureSize.y);
    float sigma = 0.3 * radius + 0.8;
    glUniform1f(prog_shadow.sigma, sigma * sigma * 2.0);
    glUniform2f(prog_shadow.step, step.x, step.y);
    glUniform4f(prog_shadow.color, color.x, color.y, color.z, color.w);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

inline void OpenGLRenderer::activateShader(const Program *shader)
{
    if (shader == m_activeShader)
        return;

    int oldCount = m_activeShader ? m_activeShader->attributeCount() : 0;
    int newCount = 0;

    if (shader) {
        newCount = shader->attributeCount();
        glUseProgram(shader->id());
    } else {
        glUseProgram(0);
    }

    // std::cout << " --- switching shader: old=" << (m_activeShader ? m_activeShader->id() : 0)
    //           << " (" << oldCount << " attr)"
    //           << " new=" << (shader ? shader->id() : 0)
    //           << " (" << newCount << " attr)"
    //           << std::endl;

    // Enable new ones
    for (int i=oldCount; i<newCount; ++i) {
        // std::cout << "    - enable " << i << estd::ndl;
        glEnableVertexAttribArray(i);
    }
    for (int i=oldCount-1; i>=newCount; --i) {
        // std::cout << "    - disable " << i << std::endl;
        glDisableVertexAttribArray(i);
    }

    m_activeShader = shader;
}

inline void OpenGLRenderer::prepass(Node *n)
{
    n->preprocess();
    switch (n->type()) {
    case Node::TextureNodeType: {
        TextureNode *tn = static_cast<TextureNode *>(n);
        if (tn->width() != 0.0f && tn->height() != 0.0f && tn->texture() != nullptr)
            ++m_numTextureNodes;
    }   break;
    case Node::RectangleNodeType: {
        RectangleNode *rn = static_cast<RectangleNode *>(n);
        if (rn->width() != 0.0f && rn->height() != 0.0f && !(rn->color().w < RENGINE_RENDERER_ALPHA_THRESHOLD))
            ++m_numRectangleNodes;
    }   break;
    case Node::TransformNodeType:
        ++m_numTransformNodes;
        if (static_cast<TransformNode *>(n)->projectionDepth() > 0)
            ++m_numTransformNodesWith3d;
        break;
    // All layered nodes take this path..
    case Node::ColorFilterNodeType:
        if (!static_cast<ColorFilterNode *>(n)->colorMatrix().isIdentity())
            ++m_numLayeredNodes;
        break;
    case Node::OpacityNodeType:
        if (static_cast<OpacityNode *>(n)->opacity() < 1)
            ++m_numLayeredNodes;
        break;
    case Node::BlurNodeType:
        if (static_cast<BlurNode *>(n)->radius() > 0) {
            ++m_numLayeredNodes;
            m_additionalQuads += 2;
        }
        break;
    case Node::ShadowNodeType:
        if (static_cast<ShadowNode *>(n)->color().w > 0) {
            ++m_numLayeredNodes;
            m_additionalQuads += 3;
        }
        break;
    case Node::RenderNodeType:
        ++m_numRenderNodes;
        break;

    default:
        // ignore...
        break;
    }

    for (Node *c = n->child(); c; c = c->sibling())
        prepass(c);
}

inline void OpenGLRenderer::build(Node *n)
{
    switch (n->type()) {
    case Node::TextureNodeType:
    case Node::RectangleNodeType: {
        rect2d geometry = static_cast<RectangleNodeBase *>(n)->geometry();

        // Skip if empty..
        if (geometry.width() == 0 || geometry.height() == 0
            || (n->type() == Node::TextureNodeType && static_cast<TextureNode *>(n)->texture() == 0)
            || (n->type() == Node::RectangleNodeType && static_cast<RectangleNode *>(n)->color().w < RENGINE_RENDERER_ALPHA_THRESHOLD))
            break;

        Element *e = m_elements + m_elementIndex;
        e->node = n;
        e->vboOffset = m_vertexIndex;
        vec2 p1 = geometry.tl;
        vec2 p2 = geometry.br;
        vec2 *v = m_vertices + m_vertexIndex;

        // std::cout << " -- building rect from " << p1 << " " << p2 << " into " << m_vertices << " " << e << std::endl;

        if (m_render3d) {
            e->z = (m_m3d * vec3((p1 + p2) / 2.0f)).z;
            projectQuad(p1, p2, v);

        } else {
            v[0] = m_m2d * p1;
            v[1] = m_m2d * vec2(p1.x, p2.y);
            v[2] = m_m2d * vec2(p2.x, p1.y);
            v[3] = m_m2d * p2;
        }
        m_vertexIndex += 4;
        m_elementIndex += 1;

        // Add to the bounding box if we're in inside a layer
        if (m_layered) {
            for (int i=0; i<4; ++i)
                m_layerBoundingBox |= v[i];
            // std::cout << " ----> bounds: " << m_layerBoundingBox << std::endl;
        }

    } break;

    case Node::TransformNodeType: {
        TransformNode *tn = static_cast<TransformNode *>(n);

        Element *e = 0;

        if (tn->projectionDepth() && !m_render3d) {
            m_render3d = true;
            m_farPlane = tn->projectionDepth();
            e = m_elements + m_elementIndex++;
            e->node = n;
            e->z = 0;
            e->projection = true;
        }

        mat4 *m = m_render3d ? &m_m3d : &m_m2d;
        mat4 old = *m;
        *m = *m * tn->matrix();

        for (Node *c = n->child(); c; c = c->sibling())
            build(c);

        // restore previous state
        *m = old;
        if (e) {
            m_render3d = false;
            m_farPlane = 0;
            e->groupSize = (m_elements + m_elementIndex) - e - 1;
        }
    } return;

    // all layered node types take this code path
    case Node::ShadowNodeType:
    case Node::BlurNodeType:
    case Node::ColorFilterNodeType:
    case Node::OpacityNodeType: {

        bool useTexture =
            (n->type() == Node::OpacityNodeType && static_cast<OpacityNode *>(n)->opacity() < 1.0f)
            || (n->type() == Node::ColorFilterNodeType && !static_cast<ColorFilterNode *>(n)->colorMatrix().isIdentity())
            || (n->type() == Node::BlurNodeType && static_cast<BlurNode *>(n)->radius() > 0)
            || (n->type() == Node::ShadowNodeType && static_cast<ShadowNode *>(n)->color().w > 0);

        bool storedTextureed = m_layered;
        Element *e = 0;
        rect2d storedBox = m_layerBoundingBox;

        if (useTexture) {
            m_layered = true;
            e = m_elements + m_elementIndex++;
            e->node = n;
            e->projection = m_render3d;
            e->layered = true;
            const float inf = std::numeric_limits<float>::infinity();
            m_layerBoundingBox = rect2d(inf, inf, -inf, -inf);
        }
        // std::cout << " -- building layered node into " << e << std::endl;

        for (Node *c = n->child(); c; c = c->sibling())
            build(c);

        if (e) {
            m_layered = storedTextureed;
            e->groupSize = (m_elements + m_elementIndex) - e - 1;
            // std::cout << "groupSize of " << e << " is " << e->groupSize << " based on: " << m_elements << " " << m_elementIndex << " " << e << std::endl;
            e->vboOffset = m_vertexIndex;
            rect2d box = m_layerBoundingBox.aligned();
            vec2 *v = m_vertices + m_vertexIndex;
            v[0] = box.tl;
            v[1] = vec2(box.left(), box.bottom());
            v[2] = vec2(box.right(), box.top());
            v[3] = box.br;
            m_vertexIndex += 4;

            if (n->type() == Node::BlurNodeType || n->type() == Node::ShadowNodeType) {
                float radius = n->type() == Node::BlurNodeType
                               ? static_cast<BlurNode *>(n)->radius()
                               : static_cast<ShadowNode *>(n)->radius();
                float t1 = box.tl.y - 1;
                float b1 = box.br.y + 1;
                vec2 tlr = box.tl - vec2(radius);
                vec2 brr = box.br + vec2(radius);
                v[ 4] = vec2(tlr.x, t1);
                v[ 5] = vec2(tlr.x, b1);
                v[ 6] = vec2(brr.x, t1);
                v[ 7] = vec2(brr.x, b1);
                v[ 8] = vec2(tlr.x, tlr.y);
                v[ 9] = vec2(tlr.x, brr.y);
                v[10] = vec2(brr.x, tlr.y);
                v[11] = vec2(brr.x, brr.y);
                m_vertexIndex += 8;

                if (n->type() == Node::ShadowNodeType) {
                    v[12] = box.tl - 1.0;
                    v[13] = vec2(box.left() - 1, box.bottom() + 1);
                    v[14] = vec2(box.right() + 1, box.top() - 1);
                    v[15] = box.br + 1;
                }
            }

            // We're a nested layer, accumulate the layered bounding box into
            // the stored one..
            if (storedTextureed)
                storedBox |= m_layerBoundingBox;

            m_layerBoundingBox = storedBox;
            if (m_render3d) {
                // Let the opacity layer's z be the average of all its children..
                float z = 0;
                for (unsigned i=0; i<=e->groupSize; ++i)
                    z += (e+i)->z;
                e->z = z / e->groupSize;
            }
        }

    } return;

    case Node::RenderNodeType: {
        Element *e = m_elements + m_elementIndex++;
        e->node = n;
        rect2d geometry = static_cast<RectangleNodeBase *>(n)->geometry();
        vec2 p1 = geometry.tl;
        vec2 p2 = geometry.br;
        // This will "kinda" work. As long as our 3d support is based on back-
        // to-front ordering of the center of primitives, we might as well
        // order render nodes back-to-front as well. The usercase is a bit
        // broken though..
        if (m_render3d)
            e->z = (m_m3d * vec3((p1 + p2) / 2.0f)).z;
        break;
    }

    default:
        break;
    }

    for (Node *c = n->child(); c; c = c->sibling())
        build(c);


}

inline void rengine_create_texture(int id, int w, int h)
{
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}


// static int recursion;

inline void OpenGLRenderer::renderToLayer(Element *e)
{
    // string space;
    // for (int i=0; i<recursion; ++i)
    //     space += "    ";
    // std::cout << space << "- doing layered rendering for: element=" << e << " node=" << e->node << std::endl;
    assert(e->layered);

    // Create the FBO
    rect2d devRect = boundingRectFor(e->vboOffset);

    // Abort the render to layer pass if the dev rect happens to be zero..
    if (devRect.width() <= 0 || devRect.height() <= 0) {
        return;
    }

    // Store current state...
    bool stored3d = m_render3d;
    bool storedTextureed = m_layered;
    GLuint storedFbo = m_fbo;
    mat4 storedProjection = m_proj;
    vec2 storedSize = m_surfaceSize;

    m_render3d |= e->projection;
    m_layered = true;


    BlurNode *blurNode = BlurNode::from(e->node);
    ShadowNode *shadowNode = ShadowNode::from(e->node);

    if (blurNode || shadowNode) {
        devRect.tl -= 1.0f;
        devRect.br += 1.0f;
    }

    // std::cout << space << " ---> from " << e->vboOffset << " " << m_vertices[e->vboOffset] << " " << m_vertices[e->vboOffset+3] << std::endl;

    m_surfaceSize = devRect.size();

    e->texture = m_texturePool.acquire();
    rengine_create_texture(e->texture, devRect.width(), devRect.height());

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e->texture, 0);

#ifndef NDEBUG
    // Only enabled in debug mode because it syncs the GL stack and takes forever..
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        logw << "FBO failed, devRect=" << devRect
             << ", dim=" << devRect.width() << "x" << devRect.height() << ", tex=" << e->texture << ", fbo=" << m_fbo << ", error="
             << std::hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        assert(false);
    }
#endif

    // Render the layered group
    m_proj = mat4::scale2D(1.0, -1.0)
             * mat4::translate2D(-1.0, 1.0)
             * mat4::scale2D(2.0f / devRect.width(), -2.0f / devRect.height())
             * mat4::translate2D(-devRect.tl.x, -devRect.tl.y);
    m_matrixState = UpdateAllPrograms;

    // std::cout << " ---> rect=" << devRect << " texture=" << e->texture << " fbo=" << m_fbo
    //           << " status=" << hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << " ok=" << GL_FRAMEBUFFER_COMPLETE
    //           << " " << m_proj << std::endl;

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    render(e + 1, e + e->groupSize + 1);

    if (blurNode || shadowNode) {
        int tmpTex = e->texture;
        e->texture = m_texturePool.acquire();
        rect2d expandedWidth = boundingRectFor(e->vboOffset + 4);
        m_proj = mat4::scale2D(1.0, -1.0)
                 * mat4::translate2D(-1.0, 1.0)
                 * mat4::scale2D(2.0f / expandedWidth.width(), -2.0f / expandedWidth.height())
                 * mat4::translate2D(-expandedWidth.tl.x, -expandedWidth.tl.y);
        m_matrixState = UpdateAllPrograms;
        rengine_create_texture(e->texture, expandedWidth.width(), expandedWidth.height());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e->texture, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, expandedWidth.width(), expandedWidth.height());
        if (blurNode) {
            drawBlurQuad(e->vboOffset + 4, tmpTex, blurNode->radius(), expandedWidth.size(), devRect.size(), vec2(1/expandedWidth.width(), 0));
            m_texturePool.release(tmpTex);
        } else if (shadowNode) {
            drawShadowQuad(e->vboOffset + 4, tmpTex, shadowNode->radius(), expandedWidth.size(), devRect.size(), vec2(1/expandedWidth.width(), 0), vec4(0, 0, 0, 1));
            e->sourceTexture = tmpTex;
        }
    }

    // Reset the GL state..
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, storedFbo);
    glDeleteFramebuffers(1, &m_fbo);

    // Reset the old state...
    m_fbo = storedFbo;
    m_render3d = stored3d;
    m_layered = storedTextureed;
    m_proj = storedProjection;
    m_matrixState = UpdateAllPrograms;
    m_surfaceSize = storedSize;

    // std::cout << space << "- layer is completed..." << std::endl;
}

/*!
    Render the elements, starting at \a first and all elements up to, but not including \a last.
 */
inline void OpenGLRenderer::render(Element *first, Element *last)
{
    // string space;
    // for (int i=0; i<recursion; ++i)
    //     space += "    ";
    // std::cout << space << "render " << first << " -> " << last - 1 << std::endl;

    // Check if we need to flatten something in this range
    if (m_numLayeredNodes > 0) {
        Element *e = first;
        // std::cout << space << "- checking layering for " << e << std::endl;
        while (e < last) {
            if (e->layered) {
                // std::cout << space << "- needs layering: " << e << std::endl;
                // ++recursion;
                renderToLayer(e);
                // --recursion;
                e = e + e->groupSize + 1;
            } else {
                ++e;
            }
        }
    }

    //
    glViewport(0, 0, m_surfaceSize.x, m_surfaceSize.y);

    Element *e = first;
    while (e < last) {
        // std::cout << space << "- render(normal) " << e << " node=(" << e->node << ") " << (e->completed ? "*done*" : "") << std::endl;
        if (e->completed) {
            ++e;
            continue;
        }

        if (e->node->type() == Node::RectangleNodeType) {
            // std::cout << space << "---> rect quad, vbo=" << e->vboOffset
            //      << " " << m_proj * m_vertices[e->vboOffset] << " " << m_proj * m_vertices[e->vboOffset+3] << std::endl;
            drawColorQuad(e->vboOffset, static_cast<RectangleNode *>(e->node)->color());
        } else if (e->node->type() == Node::TextureNodeType) {
            // std::cout << space << "---> texture quad, vbo=" << e->vboOffset << std::endl;
            const Texture *texture = static_cast<TextureNode *>(e->node)->texture();
            drawTextureQuad(e->vboOffset, texture->textureId(), 1.0f, texture->format());
        } else if (e->node->type() == Node::OpacityNodeType && e->layered && e->texture) {
            // std::cout << space << "---> layered texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << std::endl;
            drawTextureQuad(e->vboOffset, e->texture, static_cast<OpacityNode *>(e->node)->opacity());
            m_texturePool.release(e->texture);
        } else if (e->node->type() == Node::ColorFilterNodeType && e->layered && e->texture) {
            // std::cout << space << "---> layered texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << std::endl;
            drawColorFilterQuad(e->vboOffset, e->texture, static_cast<ColorFilterNode *>(e->node)->colorMatrix());
            m_texturePool.release(e->texture);
        } else if (e->node->type() == Node::BlurNodeType && e->layered && e->texture) {
            // std::cout << space << "---> blur texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << std::endl;
            BlurNode *blurNode = static_cast<BlurNode *>(e->node);
            vec2 textureSize = boundingRectFor(e->vboOffset + 4).size();
            vec2 renderSize = boundingRectFor(e->vboOffset + 8).size();
            // std::cout << " - radius: " << blurNode->radius() << " textureSize=" << textureSize << ", renderSize=" << renderSize << std::endl;
            drawBlurQuad(e->vboOffset + 8, e->texture, blurNode->radius(), renderSize, textureSize, vec2(0, 1/renderSize.y));
            m_texturePool.release(e->texture);
        } else if (e->node->type() == Node::ShadowNodeType && e->layered && e->texture) {
            // std::cout << "---> shadow texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << std::endl;
            ShadowNode *shadowNode = static_cast<ShadowNode *>(e->node);
            vec2 textureSize = boundingRectFor(e->vboOffset + 4).size();
            vec2 renderSize = boundingRectFor(e->vboOffset + 8).size();
            mat4 storedProj = m_proj;
            m_proj = m_proj * mat4::translate2D(std::round(shadowNode->offset().x), std::round(shadowNode->offset().y));
            m_matrixState |= UpdateShadowProgram;
            // std::cout << " - radius: " << shadowNode->radius() << " textureSize=" << textureSize << ", renderSize=" << renderSize << std::endl;
            drawShadowQuad(e->vboOffset + 8, e->texture, shadowNode->radius(), renderSize, textureSize, vec2(0, 1/renderSize.y), shadowNode->color());
            m_proj = storedProj;
            m_matrixState |= UpdateShadowProgram;
            drawTextureQuad(e->vboOffset + 12, e->sourceTexture);
            m_texturePool.release(e->texture);
            m_texturePool.release(e->sourceTexture);
        } else if (e->projection) {
            std::sort(e + 1, e + e->groupSize + 1);
            // std::cout << space << "---> projection, sorting range: " << (e+1) << " -> " << (e+e->groupSize) << std::endl;
        } else if (e->node->type() == Node::RenderNodeType) {
            RenderNode *rn = static_cast<RenderNode *>(e->node);
            if (rn->width() != 0 && rn->height() != 0) {
                activateShader(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                rn->render();
                setDefaultOpenGLState();
            }
        }

        e->completed = true;
        ++e;
    }
}

inline void OpenGLRenderer::setDefaultOpenGLState()
{
    // Assign our static texture coordinate buffer to attribute 1.
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Bind the vertices
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    // Set our default GL state..
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

}

inline bool OpenGLRenderer::render()
{
    if (sceneRoot() == 0) {
        logw << " - no 'sceneRoot', surely this is not what you intended?" << std::endl;
        return false;
    }

    vec4 c = fillColor();
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT);

    logd << std::endl;

    m_numLayeredNodes = 0;
    m_numTextureNodes = 0;
    m_numRectangleNodes = 0;
    m_numTransformNodes = 0;
    m_numTransformNodesWith3d = 0;
    m_numRenderNodes = 0;
    m_additionalQuads = 0;
    m_vertexIndex = 0;
    m_elementIndex = 0;
    prepass(sceneRoot());

    unsigned vertexCount = (m_numTextureNodes
                            + m_numLayeredNodes
                            + m_numRectangleNodes
                            + m_additionalQuads) * 4;
    if (vertexCount == 0)
        return true;

    m_vertices = (vec2 *) alloca(vertexCount * sizeof(vec2));
    unsigned elementCount = (m_numLayeredNodes + m_numTextureNodes + m_numRectangleNodes + m_numTransformNodesWith3d + m_numRenderNodes);
    m_elements = (Element *) alloca(elementCount * sizeof(Element));
    memset(m_elements, 0, elementCount * sizeof(Element));
    // std::cout << "render: " << m_numTextureNodes << " textures, "
    //                    << m_numRectangleNodes << " rects, "
    //                    << m_numTransformNodes << " xforms, "
    //                    << m_numTransformNodesWith3d << " xforms3D, "
    //                    << m_numLayeredNodes << " layered nodes (opacity, colorfilter, blur or shadow), "
    //                    << vertexCount * sizeof(vec2) << " bytes (" << vertexCount << " vertices), "
    //                    << elementCount * sizeof(Element) << " bytes (" << elementCount << " elements)"
    //                    << std::endl;
    build(sceneRoot());
    assert(elementCount > 0);
    assert(m_elementIndex == elementCount);
    // for (unsigned i=0; i<m_elementIndex; ++i) {
    //     const Element &e = m_elements[i];
    //     std::cout << " " << std::setw(5) << i << ": " << "element=" << &e << " node=" << e.node << " " << e.node->type() << " "
    //          << (e.projection ? "projection " : "")
    //          << "vboOffset=" << std::setw(5) << e.vboOffset << " "
    //          << "groupSize=" << std::setw(3) << e.groupSize << " "
    //          << "z=" << e.z << " " << std::endl;
    // }
    // for (unsigned i=0; i<m_vertexIndex; ++i)
    //     std::cout << "vertex[" << std::setw(5) << i << "]=" << m_vertices[i] << std::endl;

    setDefaultOpenGLState();

    // setDefaultOpenGLState will leave m_vertexBuffer bound, so we just upload into it..
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec2), m_vertices, GL_STATIC_DRAW);

    m_surfaceSize = targetSurface()->size();
    m_proj = mat4::translate2D(-1.0, 1.0)
             * mat4::scale2D(2.0f / m_surfaceSize.x, -2.0f / m_surfaceSize.y);

    assert(!m_layered);
    assert(!m_render3d);
    render(m_elements, m_elements + elementCount);

    activateShader(0);

    assert(m_fbo == 0);
    m_vertices = 0;
    m_elements = 0;

    logd << std::endl;

    return true;
}


RENGINE_END_NAMESPACE
