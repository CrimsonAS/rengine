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

#include "rengine.h"

#include <stdio.h>
#include <alloca.h>
#include <iomanip>

using namespace rengine;
using namespace std;

static const char *vsh_es_solid =
RENGINE_GLSL_HEADER
"\
attribute highp vec2 aV;                        \n\
uniform highp mat4 m;                           \n\
void main() {                                   \n\
    gl_Position = m * vec4(aV, 0, 1);           \n\
}                                               \n\
";

static const char *fsh_es_solid =
RENGINE_GLSL_HEADER
"\
uniform lowp vec4 color;                        \n\
void main() {                                   \n\
    gl_FragColor = color;                       \n\
}                                               \n\
";


static const char *vsh_es_layer =
RENGINE_GLSL_HEADER
"\
attribute highp vec2 aV;                        \n\
attribute highp vec2 aT;                        \n\
uniform highp mat4 m;                           \n\
varying highp vec2 vT;                          \n\
void main() {                                   \n\
    gl_Position = m * vec4(aV, 0, 1);           \n\
    vT = aT;                                    \n\
}                                               \n\
";

static const char *fsh_es_layer =
RENGINE_GLSL_HEADER
"\
uniform lowp sampler2D t;                       \n\
varying highp vec2 vT;                          \n\
void main() {                                   \n\
    gl_FragColor = texture2D(t, vT);            \n\
}                                               \n\
";

static const char *fsh_es_layer_alpha =
RENGINE_GLSL_HEADER
"\
uniform lowp sampler2D t;                       \n\
uniform lowp float alpha;                       \n\
varying highp vec2 vT;                          \n\
void main() {                                   \n\
    gl_FragColor = texture2D(t, vT) * alpha;    \n\
}                                               \n\
";

static const char *fsh_es_layer_colorFilter =
RENGINE_GLSL_HEADER
"\
uniform lowp sampler2D t;                       \n\
uniform lowp mat4 CM;                           \n\
varying highp vec2 vT;                          \n\
void main() {                                   \n\
    gl_FragColor = CM * texture2D(t, vT);       \n\
}                                               \n\
";

// ### Naive implementation with a lot of room for improvement...
//
// Compatibility wise, there are several older and lower-end chips that do not
// support using a uniform in a loop condition. This is not mandated by the
// GLSL spec, so it won't work everywhere.

static const char *vsh_es_layer_blur =
RENGINE_GLSL_HEADER
"\
attribute highp vec2 aV;                                            \n\
attribute highp vec2 aT;                                            \n\
uniform highp mat4 m;                                               \n\
uniform int radius;                                                 \n\
uniform highp vec4 dims;                                            \n\
varying highp vec2 vT;                                              \n\
void main() {                                                       \n\
    gl_Position = m * vec4(aV, 0, 1);                               \n\
    highp vec2 aw = dims.xy;                               \n\
    highp vec2 cw = dims.zw;                              \n\
    highp vec2 diff = (aw - cw) / aw;                     \n\
    vT = (aT - diff/2.0) * (aw / cw);                                           \n\
}                                                                   \n\
";

static const char *fsh_es_layer_blur =
RENGINE_GLSL_HEADER
"\
uniform lowp sampler2D t;                                                       \n\
uniform highp vec4 dims;                                                        \n\
uniform highp vec2 step;                                                        \n\
uniform highp float sigma;                                                      \n\
uniform int radius;                                                             \n\
varying highp vec2 vT;                                                          \n\
highp float gauss(float x) { return exp(-(x*x)/sigma); }                        \n\
void main() {                                                                   \n\
    highp float r = float(radius);                                              \n\
    highp float weights = 0.5 * gauss(r);                                       \n\
    highp vec4 result = weights * texture2D(t, vT - float(radius) * step);      \n\
    for (int i=-radius+1; i<=radius; i+=2) {                                    \n\
        highp float p1 = float(i);                                              \n\
        highp float w1 = gauss(p1);                                             \n\
        highp float p2 = float(i+1);                                            \n\
        highp float w2 = gauss(p2);                                             \n\
        highp float w = w1 + w2;                                                \n\
        highp float p = (p1 * w1 + p2 * w2) / w;                                \n\
        result += w * texture2D(t, vT + p * step);                              \n\
        weights += w;                                                           \n\
    }                                                                           \n\
    gl_FragColor = result / weights;                                            \n\
}                                                                               \n\
";

/*

    highp float r = float(radius);                                              \n\
    highp float weights = 0.5 * gauss(r);                                       \n\
    highp vec4 result = weights * texture2D(t, vT - float(radius) * step);      \n\
    for (int i=-radius+1; i<=radius; i+=2) {                                    \n\
        highp float p1 = float(i);                                              \n\
        highp float w1 = gauss(p1);                                             \n\
        highp float p2 = float(i+1);                                            \n\
        highp float w2 = gauss(p2);                                             \n\
        highp float w = w1 + w2;                                                \n\
        highp float p = (p1 * w1 + p2 * w2) / w;                                \n\
        result += w * texture2D(t, vT + p * step);                              \n\
        weights += w;                                                           \n\
    }                                                                           \n\
    gl_FragColor = result / weights;                                            \n\
*/

OpenGLRenderer::OpenGLRenderer()
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
{
    std::memset(&prog_layer, 0, sizeof(prog_layer));
    std::memset(&prog_solid, 0, sizeof(prog_solid));
    initialize();
}

OpenGLRenderer::~OpenGLRenderer()
{
    glDeleteBuffers(1, &m_texCoordBuffer);
    glDeleteBuffers(1, &m_vertexBuffer);

    assert(m_fbo == 0);
}

bool OpenGLRenderer::readPixels(int x, int y, int w, int h, unsigned *bytes)
{
    // Read line-by-line and flip it so we get what we want out..
    // Slow as hell, but this is used for autotesting, so who cares..
    for (int i=0; i<h; ++i)
        glReadPixels(x, h - i - 1, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, bytes + i * w);
    return true;
}

Layer *OpenGLRenderer::createLayerFromImageData(const vec2 &size, Layer::Format format, void *data)
{
    OpenGLTextureLayer *layer = new OpenGLTextureLayer();
    layer->setFormat(format);
    layer->upload(size.x, size.y, data);
    return layer;
}

void OpenGLRenderer::initialize()
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

    vector<const char *> attrsVT;
    attrsVT.push_back("aV");
    attrsVT.push_back("aT");

    vector<const char *> attrsV;
    attrsV.push_back("aV");

    // Default layer shader
    prog_layer.initialize(vsh_es_layer, fsh_es_layer, attrsVT);
    prog_layer.matrix = prog_layer.resolve("m");

    // Alpha layer shader
    prog_alphaLayer.initialize(vsh_es_layer, fsh_es_layer_alpha, attrsVT);
    prog_alphaLayer.matrix = prog_alphaLayer.resolve("m");
    prog_alphaLayer.alpha = prog_alphaLayer.resolve("alpha");

    // Solid color shader...
    prog_solid.initialize(vsh_es_solid, fsh_es_solid, attrsV);
    prog_solid.matrix = prog_solid.resolve("m");
    prog_solid.color = prog_solid.resolve("color");

    // Color filter shader..
    prog_colorFilter.initialize(vsh_es_layer, fsh_es_layer_colorFilter, attrsVT);
    prog_colorFilter.matrix = prog_solid.resolve("m");
    prog_colorFilter.colorMatrix = prog_colorFilter.resolve("CM");

    // Blur shader
    prog_blur.initialize(vsh_es_layer_blur, fsh_es_layer_blur, attrsVT);
    prog_blur.matrix = prog_blur.resolve("m");
    prog_blur.dims = prog_blur.resolve("dims");
    prog_blur.radius = prog_blur.resolve("radius");
    prog_blur.sigma = prog_blur.resolve("sigma");
    prog_blur.step = prog_blur.resolve("step");

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
        cout << "OpenGL" << endl
             << " - Renderer .........: " << glGetString(GL_RENDERER) << endl;
        cout << " - Version ..........: " << glGetString(GL_VERSION) << endl;
        cout << " - R/G/B/A ..........: " << r << " " << g << " " << b << " " << a << endl;
        cout << " - Depth/Stencil ....: " << d << " " << s << endl;
        cout << " - Samples ..........: " << samples << endl;
        cout << " - Max Texture Size .: " << maxTexSize << endl;
        cout << " - Extensions .......: " << glGetString(GL_EXTENSIONS) << endl;
    }
#endif

}

/*!

    Draws a quad using the 'solid' program. \a v is a vector of 8 floats,
    composed of four interleaved x/y points. \a c is the color.

 */
void OpenGLRenderer::drawColorQuad(unsigned offset, const vec4 &c)
{
    activateShader(&prog_solid);
    ensureMatrixUpdated(UpdateSolidProgram, &prog_solid);
    glUniform4f(prog_solid.color, c.x * c.w, c.y * c.w, c.z * c.w, c.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawColorFilterQuad(unsigned offset, GLuint texId, const mat4 &matrix)
{
    activateShader(&prog_colorFilter);
    ensureMatrixUpdated(UpdateColorFilterProgram, &prog_colorFilter);
    glUniformMatrix4fv(prog_colorFilter.colorMatrix, 1, true, matrix.m);
    // cout << prog_colorFilter.colorMatrix << matrix;
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawTextureQuad(unsigned offset, GLuint texId, float opacity)
{
    if (opacity == 1) {
        activateShader(&prog_layer);
        ensureMatrixUpdated(UpdateLayerProgram, &prog_layer);
    } else {
        activateShader(&prog_alphaLayer);
        ensureMatrixUpdated(UpdateAlphaLayerProgram, &prog_alphaLayer);
        glUniform1f(prog_alphaLayer.alpha, opacity);
    }

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawBlurQuad(unsigned offset, GLuint texId, int radius, const vec2 &renderSize, const vec2 &textureSize, const vec2 &step)
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

void OpenGLRenderer::activateShader(const Program *shader)
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

    // cout << " --- switching shader: old=" << (m_activeShader ? m_activeShader->id() : 0)
    //      << " (" << oldCount << " attr)"
    //      << " new=" << (shader ? shader->id() : 0)
    //      << " (" << newCount << " attr)"
    //      << endl;

    // Enable new ones
    for (int i=oldCount; i<newCount; ++i) {
        // cout << "    - enable " << i << endl;
        glEnableVertexAttribArray(i);
    }
    for (int i=oldCount-1; i>=newCount; --i) {
        // cout << "    - disable " << i << endl;
        glDisableVertexAttribArray(i);
    }

    m_activeShader = shader;
}

void OpenGLRenderer::prepass(Node *n)
{
    n->preprocess();
    switch (n->type()) {
    case Node::LayerNodeType:
        ++m_numTextureNodes;
        break;
    case Node::RectangleNodeType:
        ++m_numRectangleNodes;
        break;
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
        if (static_cast<OpacityNode *>(n)->opacity() < 1.0f)
            ++m_numLayeredNodes;
        break;
    case Node::BlurNodeType:
        if (static_cast<BlurNode *>(n)->radius() > 0) {
            ++m_numLayeredNodes;
            m_additionalQuads += 2;
        }
        break;
    default:
        // ignore...
        break;
    }

    for (Node *c = n->child(); c; c = c->sibling())
        prepass(c);
}

void OpenGLRenderer::build(Node *n)
{
    switch (n->type()) {
    case Node::LayerNodeType:
    case Node::RectangleNodeType: {
        const rect2d &geometry = n->type() == Node::LayerNodeType
                                 ? static_cast<LayerNode *>(n)->geometry()
                                 : static_cast<RectangleNode *>(n)->geometry();
        Element *e = m_elements + m_elementIndex;
        e->node = n;
        e->vboOffset = m_vertexIndex;
        const vec2 &p1 = geometry.tl;
        const vec2 &p2 = geometry.br;
        vec2 *v = m_vertices + m_vertexIndex;

        // cout << " -- building rect from " << p1 << " " << p2 << " into " << m_vertices << " " << e << endl;

        if (m_render3d) {
            e->z = (m_m3d * vec3((p1 + p2) / 2.0f)).z;
            projectQuad(p1, p2, v);

        } else {
            vec2 a = m_m2d * p1;
            vec2 b = m_m2d * p2;
            v[0] = vec2(a.x, a.y);
            v[1] = vec2(a.x, b.y);
            v[2] = vec2(b.x, a.y);
            v[3] = vec2(b.x, b.y);
        }
        m_vertexIndex += 4;
        m_elementIndex += 1;

        // Add to the bounding box if we're in inside a layer
        if (m_layered) {
            for (int i=0; i<4; ++i)
                m_layerBoundingBox |= v[i];
            // cout << " ----> bounds: " << m_layerBoundingBox << endl;
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
    case Node::BlurNodeType:
    case Node::ColorFilterNodeType:
    case Node::OpacityNodeType: {

        bool useLayer =
            (n->type() == Node::OpacityNodeType && static_cast<OpacityNode *>(n)->opacity() < 1.0f)
            || (n->type() == Node::ColorFilterNodeType && !static_cast<ColorFilterNode *>(n)->colorMatrix().isIdentity())
            || (n->type() == Node::BlurNodeType && static_cast<BlurNode *>(n)->radius() > 0);

        bool storedLayered = m_layered;
        Element *e = 0;
        rect2d storedBox = m_layerBoundingBox;

        if (useLayer) {
            m_layered = true;
            e = m_elements + m_elementIndex++;
            e->node = n;
            e->projection = m_render3d;
            e->layered = true;
            const float inf = numeric_limits<float>::infinity();
            m_layerBoundingBox = rect2d(inf, inf, -inf, -inf);
        }
        // cout << " -- building layered node into " << e << endl;

        for (Node *c = n->child(); c; c = c->sibling())
            build(c);

        if (e) {
            m_layered = storedLayered;
            e->groupSize = (m_elements + m_elementIndex) - e - 1;
            // cout << "groupSize of " << e << " is " << e->groupSize << " based on: " << m_elements << " " << m_elementIndex << " " << e << endl;
            e->vboOffset = m_vertexIndex;
            rect2d box = m_layerBoundingBox.aligned();
            vec2 *v = m_vertices + m_vertexIndex;
            v[0] = box.tl;
            v[1] = vec2(box.left(), box.bottom());
            v[2] = vec2(box.right(), box.top());
            v[3] = box.br;
            m_vertexIndex += 4;

            if (BlurNode *blurNode = Node::from<BlurNode>(n)) {
                float t1 = box.tl.y - 1;
                float b1 = box.br.y + 1;
                vec2 tlr = box.tl - vec2(blurNode->radius());
                vec2 brr = box.br + vec2(blurNode->radius());
                v[ 4] = vec2(tlr.x, t1);
                v[ 5] = vec2(tlr.x, b1);
                v[ 6] = vec2(brr.x, t1);
                v[ 7] = vec2(brr.x, b1);
                v[ 8] = vec2(tlr.x, tlr.y);
                v[ 9] = vec2(tlr.x, brr.y);
                v[10] = vec2(brr.x, tlr.y);
                v[11] = vec2(brr.x, brr.y);
                m_vertexIndex += 8;
            }

            // We're a nested layer, accumulate the layered bounding box into
            // the stored one..
            if (storedLayered)
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

    default:
        break;
    }

    for (Node *c = n->child(); c; c = c->sibling())
        build(c);


}

static void rengine_create_texture(int id, int w, int h)
{
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}


// static int recursion;

void OpenGLRenderer::renderToLayer(Element *e)
{
    // string space;
    // for (int i=0; i<recursion; ++i)
    //     space += "    ";
    // cout << space << "- doing layered rendering for: element=" << e << " node=" << e->node << endl;
    assert(e->layered);

    // Store current state...
    bool stored3d = m_render3d;
    bool storedLayered = m_layered;
    GLuint storedFbo = m_fbo;
    mat4 storedProjection = m_proj;
    vec2 storedSize = m_surfaceSize;

    m_render3d |= e->projection;
    m_layered = true;

    BlurNode *blurNode = Node::from<BlurNode>(e->node);

    // Create the FBO
    rect2d devRect(m_vertices[e->vboOffset], m_vertices[e->vboOffset + 3]);
    assert(devRect.width() >= 0);
    assert(devRect.height() >= 0);
    if (blurNode) {
        devRect.tl -= 1.0f;
        devRect.br += 1.0f;
    }

    // cout << space << " ---> from " << e->vboOffset << " " << m_vertices[e->vboOffset] << " " << m_vertices[e->vboOffset+3] << endl;

    m_surfaceSize = devRect.size();

    e->texture = m_texturePool.acquire();
    rengine_create_texture(e->texture, devRect.width(), devRect.height());

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e->texture, 0);

#ifndef NDEBUG
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "OpenGLRenderer::renderToLayer: FBO failed, devRect=" << devRect
             << ", dim=" << devRect.width() << "x" << devRect.height() << ", tex=" << e->texture << ", fbo=" << m_fbo << ", error="
             << hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
        assert(false);
    }
#endif

    // Render the layered group
    m_proj = mat4::scale2D(1.0, -1.0)
             * mat4::translate2D(-1.0, 1.0)
             * mat4::scale2D(2.0f / devRect.width(), -2.0f / devRect.height())
             * mat4::translate2D(-devRect.tl.x, -devRect.tl.y);
    m_matrixState = UpdateAllPrograms;

    // cout << " ---> rect=" << devRect << " texture=" << e->texture << " fbo=" << m_fbo
    //      << " status=" << hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << " ok=" << GL_FRAMEBUFFER_COMPLETE
    //      << " " << m_proj << endl;

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    render(e + 1, e + e->groupSize + 1);

    if (blurNode) {
        int tmpTex = e->texture;
        e->texture = m_texturePool.acquire();
        rect2d expandedWidth(m_vertices[e->vboOffset + 4], m_vertices[e->vboOffset + 4 + 3]);
        m_proj = mat4::scale2D(1.0, -1.0)
                 * mat4::translate2D(-1.0, 1.0)
                 * mat4::scale2D(2.0f / expandedWidth.width(), -2.0f / expandedWidth.height())
                 * mat4::translate2D(-expandedWidth.tl.x, -expandedWidth.tl.y);
        m_matrixState = UpdateAllPrograms;
        rengine_create_texture(e->texture, expandedWidth.width(), expandedWidth.height());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e->texture, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, expandedWidth.width(), expandedWidth.height());
        drawBlurQuad(e->vboOffset + 4, tmpTex, blurNode->radius(), expandedWidth.size(), devRect.size(), vec2(1/expandedWidth.width(), 0));
        m_texturePool.release(tmpTex);
    }

    // Reset the GL state..
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, storedFbo);
    glDeleteFramebuffers(1, &m_fbo);

    // Reset the old state...
    m_fbo = storedFbo;
    m_render3d = stored3d;
    m_layered = storedLayered;
    m_proj = storedProjection;
    m_matrixState = UpdateAllPrograms;
    m_surfaceSize = storedSize;

    // cout << space << "- layer is completed..." << endl;
}

/*!
    Render the elements, starting at \a first and all elements up to, but not including \a last.
 */
void OpenGLRenderer::render(Element *first, Element *last)
{
    // string space;
    // for (int i=0; i<recursion; ++i)
    //     space += "    ";
    // cout << space << "render " << first << " -> " << last - 1 << endl;

    // Check if we need to flatten something in this range
    if (m_numLayeredNodes > 0) {
        Element *e = first;
        // cout << space << "- checking layering for " << e << endl;
        while (e < last) {
            if (e->layered) {
                // cout << space << "- needs layering: " << e << endl;
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
        // cout << space << "- render(normal) " << e << " node=(" << e->node << ") " << (e->completed ? "*done*" : "") << endl;
        if (e->completed) {
            ++e;
            continue;
        }

        if (e->node->type() == Node::RectangleNodeType) {
            // cout << space << "---> rect quad, vbo=" << e->vboOffset
            //      << " " << m_proj * m_vertices[e->vboOffset] << " " << m_proj * m_vertices[e->vboOffset+3] << endl;
            drawColorQuad(e->vboOffset, static_cast<RectangleNode *>(e->node)->color());
        } else if (e->node->type() == Node::LayerNodeType) {
            // cout << space << "---> texture quad, vbo=" << e->vboOffset << endl;
            drawTextureQuad(e->vboOffset, static_cast<LayerNode *>(e->node)->layer()->textureId());
        } else if (e->node->type() == Node::OpacityNodeType && e->layered) {
            // cout << space << "---> layered texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << endl;
            drawTextureQuad(e->vboOffset, e->texture, static_cast<OpacityNode *>(e->node)->opacity());
            m_texturePool.release(e->texture);
        } else if (e->node->type() == Node::ColorFilterNodeType && e->layered) {
            // cout << space << "---> layered texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << endl;
            drawColorFilterQuad(e->vboOffset, e->texture, static_cast<ColorFilterNode *>(e->node)->colorMatrix());
            m_texturePool.release(e->texture);
        } else if (e->node->type() == Node::BlurNodeType && e->layered) {
            // cout << space << "---> blur texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << endl;
            BlurNode *blurNode = static_cast<BlurNode *>(e->node);
            vec2 textureSize = m_vertices[e->vboOffset + 4 + 3] - m_vertices[e->vboOffset + 4] + 2.0;
            vec2 renderSize = m_vertices[e->vboOffset + 8 + 3] - m_vertices[e->vboOffset + 8];
            // cout << " - radius: " << blurNode->radius() << " textureSize=" << textureSize << ", renderSize=" << renderSize << endl;
            drawBlurQuad(e->vboOffset + 8, e->texture, blurNode->radius(), renderSize, textureSize, vec2(0, 1/renderSize.y));
            m_texturePool.release(e->texture);
        } else if (e->projection) {
            std::sort(e + 1, e + e->groupSize + 1);
            // cout << space << "---> projection, sorting range: " << (e+1) << " -> " << (e+e->groupSize) << endl;
        }

        e->completed = true;
        ++e;
    }
}

bool OpenGLRenderer::render()
{
    if (sceneRoot() == 0) {
        cout << __PRETTY_FUNCTION__ << " - no 'sceneRoot', surely this is not what you intended?" << endl;
        return false;
    }

    m_numLayeredNodes = 0;
    m_numTextureNodes = 0;
    m_numRectangleNodes = 0;
    m_numTransformNodes = 0;
    m_numTransformNodesWith3d = 0;
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
    unsigned elementCount = (m_numLayeredNodes + m_numTextureNodes + m_numRectangleNodes + m_numTransformNodesWith3d);
    m_elements = (Element *) alloca(elementCount * sizeof(Element));
    memset(m_elements, 0, elementCount * sizeof(Element));
    // cout << "render: " << m_numTextureNodes << " layers, "
    //                    << m_numRectangleNodes << " rects, "
    //                    << m_numTransformNodes << " xforms, "
    //                    << m_numTransformNodesWith3d << " xforms3D, "
    //                    << m_numLayeredNodes << " opacites, "
    //                    << vertexCount * sizeof(vec2) << " bytes (" << vertexCount << " vertices), "
    //                    << elementCount * sizeof(Element) << " bytes (" << elementCount << " elements)"
    //                    << endl;
    build(sceneRoot());
    assert(elementCount > 0);
    assert(m_elementIndex == elementCount);
    // for (unsigned i=0; i<m_elementIndex; ++i) {
    //     const Element &e = m_elements[i];
    //     cout << " " << setw(5) << i << ": " << "element=" << &e << " node=" << e.node << " " << e.node->type() << " "
    //          << (e.projection ? "projection " : "")
    //          << "vboOffset=" << setw(5) << e.vboOffset << " "
    //          << "groupSize=" << setw(3) << e.groupSize << " "
    //          << "z=" << e.z << " " << endl;
    // }
    // for (unsigned i=0; i<m_vertexIndex; ++i)
    //     cout << "vertex[" << setw(5) << i << "]=" << m_vertices[i] << endl;

    // Assign our static texture coordinate buffer to attribute 1.
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Upload the vertices for this frame
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec2), m_vertices, GL_STATIC_DRAW);

    m_surfaceSize = targetSurface()->size();

    vec4 c = fillColor();
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    m_proj = mat4::translate2D(-1.0, 1.0)
             * mat4::scale2D(2.0f / m_surfaceSize.x, -2.0f / m_surfaceSize.y);

    assert(!m_layered);
    assert(!m_render3d);
    render(m_elements, m_elements + elementCount);

    activateShader(0);

    assert(m_fbo == 0);
    m_vertices = 0;
    m_elements = 0;

    return true;
}

