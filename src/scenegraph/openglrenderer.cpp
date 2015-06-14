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
    gl_FragColor = texture2D(t, vT) + vec4(0.1);            \n\
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

OpenGLRenderer::OpenGLRenderer()
    : m_numLayeredNodes(0)
    , m_numTextureNodes(0)
    , m_numRectangleNodes(0)
    , m_numTransformNodes(0)
    , m_numTransformNodesWith3d(0)
    , m_vertexIndex(0)
    , m_elementIndex(0)
    , m_vertices(0)
    , m_elements(0)
    , m_farPlane(0)
    , m_activeShader(0)
    , m_texCoordBuffer(0)
    , m_vertexBuffer(0)
    , m_fbo(0)
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

bool OpenGLRenderer::readPixels(int x, int y, int w, int h, unsigned char *bytes)
{
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
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

    { // Default layer shader
        vector<const char *> attrs;
        attrs.push_back("aV");
        attrs.push_back("aT");
        prog_layer.initialize(vsh_es_layer, fsh_es_layer, attrs);
        prog_layer.matrix = prog_layer.resolve("m");
    }

    { // Alpha layer shader
        vector<const char *> attrs;
        attrs.push_back("aV");
        attrs.push_back("aT");
        prog_alphaLayer.initialize(vsh_es_layer, fsh_es_layer_alpha, attrs);
        prog_alphaLayer.matrix = prog_alphaLayer.resolve("m");
        prog_alphaLayer.alpha = prog_alphaLayer.resolve("alpha");
    }

    { // Solid color shader...
        vector<const char *> attrs;
        attrs.push_back("aV");
        prog_solid.initialize(vsh_es_solid, fsh_es_solid, attrs);
        prog_solid.matrix = prog_solid.resolve("m");
        prog_solid.color = prog_solid.resolve("color");
    }

#ifdef RENGINE_LOG_INFO
    static bool logged = false;
    if (!logged) {
        logged = true;
        int samples, maxTexSize;
        int r, g, b, a, d, s;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glGetIntegerv(GL_SAMPLES, &samples);
        glGetIntegerv(GL_RED_BITS, r);
        glGetIntegerv(GL_GREEN_BITS, g);
        glGetIntegerv(GL_BLUE_BITS, b);
        glGetIntegerv(GL_ALPHA_BITS, a);
        glGetIntegerv(GL_DEPTH_BITS, d);
        glGetIntegerv(GL_STENCIL_BITS, s);
        cout << "OpenGL" << endl
             << " - Renderer .........: " << glGetString(GL_RENDERER) << endl;
        cout << " - Version ..........: " << glGetString(GL_VERSION) << endl;
        cout << " - R/G/B/A ..........: " << r << " " << g << " " << " " << b << " " << a << endl;
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

    glUniform4f(prog_solid.color, c.x * c.w, c.y * c.w, c.z * c.w, c.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawTextureQuad(unsigned offset, GLuint texId, float opacity)
{
    if (opacity == 1)
        activateShader(&prog_layer);
    else {
        activateShader(&prog_alphaLayer);
        glUniform1f(prog_alphaLayer.alpha, opacity);
    }

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
        glUniformMatrix4fv(shader->matrix, 1, true, m_proj.m);
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
    case Node::OpacityNodeType: ++m_numLayeredNodes; break;
    case Node::LayerNodeType: ++m_numTextureNodes; break;
    case Node::RectangleNodeType: ++m_numRectangleNodes; break;
    case Node::TransformNodeType:
        ++m_numTransformNodes;
        if (static_cast<TransformNode *>(n)->projectionDepth() > 0)
            ++m_numTransformNodesWith3d;
        break;
    default:
        // ignore...
        break;
    }

    for (auto c : n->children())
        prepass(c);
}

void OpenGLRenderer::build(Node *n)
{
    switch (n->type()) {
    case Node::LayerNodeType:
    case Node::RectangleNodeType: {
        RectangleNode *rn = static_cast<RectangleNode *>(n);
        Element *e = m_elements + m_elementIndex;
        e->node = n;
        e->vboOffset = m_vertexIndex;
        vec2 p1 = rn->position();
        vec2 p2 = rn->size() + rn->position();
        vec2 *v = m_vertices + m_vertexIndex;

        if (m_render3d) {
            e->z = (m_m3d * vec3((p1 + p2) / 2.0f)).z;
            projectQuad(p1, p2, v);

        } else {
            p1 = m_m2d * p1;
            p2 = m_m2d * p2;
            v[0] = vec2(p1.x, p1.y);
            v[1] = vec2(p1.x, p2.y);
            v[2] = vec2(p2.x, p1.y);
            v[3] = vec2(p2.x, p2.y);
        }
        m_vertexIndex += 4;
        m_elementIndex += 1;

        // Add to the bounding box if we're in inside a layer
        if (m_layered) {
            for (int i=0; i<4; ++i)
                m_layerBoundingBox |= v[i];
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

        for (auto c : n->children())
            build(c);

        // restore previous state
        *m = old;
        if (e) {
            m_render3d = false;
            m_farPlane = 0;
            e->groupSize = (m_elements + m_elementIndex) - e;
        }
    } return;

    case Node::OpacityNodeType: {
        OpacityNode *on = static_cast<OpacityNode *>(n);

        bool storedLayered = m_layered;
        Element *e = 0;
        rect2d storedBox = m_layerBoundingBox;
        if (on->opacity() < 1.0) {
            m_layered = true;
            e = m_elements + m_elementIndex++;
            e->node = on;
            e->projection = m_render3d;
            e->layered = true;
            const float inf = numeric_limits<float>::infinity();
            m_layerBoundingBox = rect2d(inf, inf, -inf, -inf);
        }

        for (auto c : n->children())
            build(c);

        if (e) {
            m_layered = storedLayered;
            e->groupSize = (m_elements + m_elementIndex) - e;
            e->vboOffset = m_vertexIndex;
            rect2d box = m_layerBoundingBox.aligned();
            vec2 *v = m_vertices + m_vertexIndex;
            v[0] = box.tl;
            v[1] = vec2(box.left(), box.bottom());
            v[2] = vec2(box.right(), box.top());
            v[3] = box.br;
            m_vertexIndex += 4;

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

    for (auto c : n->children())
        build(c);


}


void OpenGLRenderer::renderToLayer(Element *e)
{
    // cout << " ---> doing layered rendering for: element=" << e << " node=" << e->node << endl;
    assert(e->layered);

    // Store current state...
    bool stored3d = m_render3d;
    bool storedLayered = m_layered;
    GLuint storedFbo = m_fbo;
    mat4 storedProjection = m_proj;

    m_render3d |= e->projection;
    m_layered = true;

    // Create the FBO
    rect2d devRect(m_vertices[e->vboOffset], m_vertices[e->vboOffset + 3]);
    int w = devRect.width();
    int h = devRect.height();
    e->texture = m_texturePool.acquire();
    glBindTexture(GL_TEXTURE_2D, e->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e->texture, 0);
    // cout << "    -> FBO status: " << hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << end;
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    // Render the layered group
    m_proj = mat4::scale2D(1.0, -1.0)
             * mat4::translate2D(-1.0, 1.0)
             * mat4::scale2D(2.0f / w, -2.0f / h)
             * mat4::translate2D(-devRect.tl.x, -devRect.tl.y);

    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    render(e + 1, e + e->groupSize);

    // Reset the GL state..
    glViewport(0, 0, m_surfaceSize.x, m_surfaceSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, storedFbo);
    glDeleteFramebuffers(1, &m_fbo);

    // Reset the old state...
    m_fbo = storedFbo;
    m_render3d = stored3d;
    m_layered = storedLayered;
    m_proj = storedProjection;
}


void OpenGLRenderer::render(Element *first, Element *last)
{
    // Check if we need to flatten something in this range
    if (m_numLayeredNodes > 0) {
        // cout << " - render(layered) " << first->node << endl;
        Element *e = first;
        while (e < last) {
            if (e->layered) {
                renderToLayer(e);
                e = e + e->groupSize;
            } else {
                // cout << " ---> skipping" << endl;
                ++e;
            }
        }
    }

    Element *e = first;
    while (e < last) {
        // cout << " - render(normal) " << e->node << " " << (e->completed ? "*done*" : "") << endl;
        if (e->completed) {
            ++e;
            continue;
        }

        if (e->node->type() == Node::RectangleNodeType) {
            // cout << " ---> rect quad, vbo=" << e->vboOffset << endl;
            drawColorQuad(e->vboOffset, static_cast<RectangleNode *>(e->node)->color());
        } else if (e->node->type() == Node::LayerNodeType) {
            // cout << " ---> texture quad, vbo=" << e->vboOffset << endl;
            drawTextureQuad(e->vboOffset, static_cast<LayerNode *>(e->node)->layer()->textureId());
        } else if (e->layered) {
            // cout << " ---> layered texture quad, vbo=" << e->vboOffset << " texture=" << e->texture << endl;
            drawTextureQuad(e->vboOffset, e->texture, Node::from<OpacityNode>(e->node)->opacity());
            m_texturePool.release(e->texture);
        } else if (e->projection) {
            std::sort(e + 1, e + e->groupSize);
            // cout << " ---> projection, sorting range: " << (e+1) << " -> " << (e+e->groupSize) << endl;
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
    m_vertexIndex = 0;
    m_elementIndex = 0;
    prepass(sceneRoot());

    unsigned vertexCount = (m_numTextureNodes + m_numLayeredNodes + m_numRectangleNodes) * 4;
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

    // Assign our static texture coordinate buffer to attribute 1.
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Upload the vertices for this frame
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec2), m_vertices, GL_STATIC_DRAW);

    m_surfaceSize = targetSurface()->size();
    glViewport(0, 0, m_surfaceSize.x, m_surfaceSize.y);

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

