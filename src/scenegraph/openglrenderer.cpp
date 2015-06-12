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
    glDeleteProgram(prog_layer.id());
    glDeleteProgram(prog_solid.id());
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

    glUniform4f(prog_solid.color, c.x, c.y, c.z, c.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawTextureQuad(unsigned offset, GLuint texId)
{
    activateShader(&prog_layer);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) (offset * sizeof(vec2)));
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::activateShader(const OpenGLShaderProgram *shader)
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

    // Enable new ones
    for (int i=oldCount; i<newCount; ++i)
        glEnableVertexAttribArray(i);
    for (int i=oldCount-1; i>=newCount; --i)
        glDisableVertexAttribArray(i);
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
        e->range = m_vertexIndex;
        e->layered = m_layered;
        e->projection = m_render3d;
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

        for (auto c : n->children())
            build(c);

    } break;

    case Node::TransformNodeType: {
        TransformNode *tn = static_cast<TransformNode *>(n);

        bool entered3d = false;
        unsigned index = m_elementIndex;

        if (tn->projectionDepth() && !m_render3d) {
            entered3d = true;
            m_render3d = true;
            m_farPlane = tn->projectionDepth();
            Element *e = m_elements + m_elementIndex++;
            e->node = n;
            e->z = 0;
            e->projection = true;
            e->layered = m_layered;
        }

        mat4 *m = m_render3d ? &m_m3d : &m_m2d;
        mat4 old = *m;
        *m = *m * tn->matrix();

        for (auto c : n->children())
            build(c);

        // restore previous state
        *m = old;
        if (entered3d) {
            m_render3d = false;
            m_farPlane = 0;
            m_elements[index].range = m_elementIndex-1;
        }
    } break;

    case Node::OpacityNodeType: {
        OpacityNode *on = static_cast<OpacityNode *>(n);

        bool enteredLayer = false;
        unsigned index = m_elementIndex;
        if (on->opacity() < 1.0) {
            enteredLayer = true;
            m_layered = true;
            Element *e = m_elements + m_elementIndex++;
            e->node = on;
            e->projection = m_render3d;
            e->layered = true;
        }

        for (auto c : n->children())
            build(c);

        if (enteredLayer) {
            Element &e = m_elements[index];
            e.range = m_elementIndex-1;
            if (m_render3d)
                e.z = (m_m3d * vec3(e.bounds.center())).z;
        }

    } break;

    default:
        for (auto c : n->children())
            build(c);
        break;
    }
}



void OpenGLRenderer::render(unsigned first, unsigned last)
{
    if (m_elements[first].projection) {
        std::sort(m_elements+first, m_elements+last+1);
    }

    while (first <= last) {
        const Element &e = m_elements[first];

        if (e.node->type() == Node::RectangleNodeType) {
            drawColorQuad(e.range, static_cast<RectangleNode *>(e.node)->color());
        } else if (e.node->type() == Node::LayerNodeType) {
            drawTextureQuad(e.range, static_cast<LayerNode *>(e.node)->layer()->textureId());
        }

        ++first;
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
    //     cout << " " << i << ": " << e.node->type() << " "
    //          << (e.projection ? "projection " : "orthogonal ")
    //          << "range=" << e.range << " "
    //          << "z=" << e.z << endl;
    // }

    // Assign our static texture coordinate buffer to attribute 1.
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Upload the vertices for this frame
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec2), m_vertices, GL_STATIC_DRAW);

    vec2 surfaceSize = targetSurface()->size();
    glViewport(0, 0, surfaceSize.x, surfaceSize.y);

    vec4 c = fillColor();
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mat4 proj = mat4::translate2D(-1.0, 1.0)
                * mat4::scale2D(2.0f / surfaceSize.x, -2.0f / surfaceSize.y);
    // Push the screenspace projection matrix to the programs. We handle
    glUseProgram(prog_layer.id());
    glUniformMatrix4fv(prog_layer.matrix, 1, true, proj.m);
    glUseProgram(prog_solid.id());
    glUniformMatrix4fv(prog_solid.matrix, 1, true, proj.m);

    render(0, elementCount-1);

    activateShader(0);

    m_vertices = 0;
    m_elements = 0;

    return true;
}

