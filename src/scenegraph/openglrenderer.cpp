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

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
*/

#include "rengine.h"

#include <stdio.h>

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
    : m_gl(0)
    , m_activeShader(0)
{
    std::memset(&prog_layer, 0, sizeof(prog_layer));
    std::memset(&prog_solid, 0, sizeof(prog_solid));
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
    m_gl->makeCurrent(targetSurface());

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
}

bool OpenGLRenderer::render()
{
    assert(m_states.empty());

    if (sceneRoot() == 0) {
        cout << __PRETTY_FUNCTION__ << " - no 'sceneRoot', surely this is not what you intended?" << endl;
        return false;
    }
    m_gl->makeCurrent(targetSurface());

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
    glUseProgram(prog_layer.id());
    glUniformMatrix4fv(prog_layer.matrix, 1, true, proj.m);
    glUseProgram(prog_solid.id());
    glUniformMatrix4fv(prog_solid.matrix, 1, true, proj.m);

    m_states.push_back(RenderState());
    state()->matrices.push(proj);

    render(sceneRoot());

    m_states.pop_back();
    assert(m_states.empty());

    activateShader(0);

    m_gl->swapBuffers(targetSurface());

    return true;
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

/*!

    Draws a quad using the 'solid' program. \a v is a vector of 8 floats,
    composed of four interleaved x/y points. \a c is the color.

 */
void OpenGLRenderer::drawColorQuad(const float *v, const vec4 &c)
{
    activateShader(&prog_solid);

    glUniform4f(prog_solid.color, c.x, c.y, c.z, c.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, v);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawTextureQuad(const float *v, GLuint texId)
{
    activateShader(&prog_layer);

    const float tv[] = { 0, 0, 0, 1, 1, 0, 1, 1 };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, v);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tv);
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::render(Node *n)
{
    if (LayerNode *ln = Node::from<LayerNode>(n)) {
        const vec2 &p1 = state()->matrices.top() * ln->position();
        const vec2 &p2 = state()->matrices.top() * (ln->size() + ln->position());
        const float data[] = { p1.x, p1.y,
                               p1.x, p2.y,
                               p2.x, p1.y,
                               p2.x, p2.y };

        Layer *l = ln->layer();
        assert(l);
        assert(l->textureId());
        drawTextureQuad(data, l->textureId());

    } else if (RectangleNode *rn = Node::from<RectangleNode>(n)) {
        const vec2 &p1 = state()->matrices.top() * rn->position();
        const vec2 &p2 = state()->matrices.top() * (rn->size() + rn->position());
        const float data[] = { p1.x, p1.y,
                               p1.x, p2.y,
                               p2.x, p1.y,
                               p2.x, p2.y };
        drawColorQuad(data, rn->color());

    } else if (TransformNode *tn = Node::from<TransformNode>(n)) {
        if (tn->projectionDepth() > 0) {
            m_states.push_back(RenderState());
            state()->farPlane = tn->projectionDepth();
            state()->matrices.push(tn->matrix());

            for (auto child : n->children())
                gatherNodes3D(child);

            render3D();

            m_states.pop_back();

        } else {
            state()->matrices.push(tn->matrix());
            for (auto it : n->children())
                render(it);
            state()->matrices.pop();
        }
        return;

    } else if (OpacityNode *on = Node::from<OpacityNode>(n)) {
        cout << "opacity node at: " << on->opacity() << endl;
    }

    for (auto it : n->children())
        render(it);
}


void OpenGLRenderer::gatherNodes3D(Node *n)
{
    assert(m_states.size() > 1); // base state won't have projection, so we must have at least two...
    assert(state()->farPlane > 0); // pre-req for entering into 3D rendering in the first place.

    if (n->type() == Node::LayerNodeType || n->type() == Node::RectangleNodeType) {
        RectangleNode *rn = static_cast<RectangleNode *>(n);
        const vec2 &p1 = rn->position();
        const vec2 &p2 = rn->size() + rn->position();
        NodeToRender ntr;
        ntr.node = n;
        ntr.z = (state()->matrices.top() * vec3((p1 + p2) / 2.0f)).z;
        projectQuad(p1, p2, ntr.vertices);
        state()->nodes.push_back(ntr);

    } else if (TransformNode *tn = Node::from<TransformNode>(n)) {
        if (tn->projectionDepth() > 0) {
            cout << "Nested projection depths are currently not supported, ignoring..." << endl;
        }

        state()->push(tn->matrix());
        for (auto child : n->children())
            gatherNodes3D(child);
        state()->pop();

        return;
    }

    for (auto child : n->children())
        gatherNodes3D(child);

}



void OpenGLRenderer::render3D()
{
    sort(state()->nodes.begin(), state()->nodes.end());

    for (const auto &n : state()->nodes) {
        if (LayerNode *ln = Node::from<LayerNode>(n.node)) {
            Layer *l = ln->layer();
            assert(l);
            assert(l->textureId());
            drawTextureQuad(n.vertices, l->textureId());
        } else if (RectangleNode *rn = Node::from<RectangleNode>(n.node)) {
            drawColorQuad(n.vertices, rn->color());
        }
    }
}