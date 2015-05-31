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
    , m_currentTransformNode(0)
{
    memset(&prog_layer, 0, sizeof(prog_layer));
    memset(&prog_solid, 0, sizeof(prog_solid));
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

    glDepthRange(-1000.0, 1000.0);
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
    glUniformMatrix4fv(prog_layer.matrix, 1, true, proj.m);

    assert(m_matrixStack.empty());
    m_matrixStack.push(mat4());

    render(sceneRoot());

    m_matrixStack.pop();
    assert(m_matrixStack.empty());

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
void OpenGLRenderer::drawColorQuad(float *v, const vec4 &c)
{
    activateShader(&prog_solid);

    glUniform4f(prog_solid.color, c.x, c.y, c.z, c.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, v);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLRenderer::drawTextureQuad(float *v, GLuint texId)
{
    activateShader(&prog_layer);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), v);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), v + 2);
    glBindTexture(GL_TEXTURE_2D, texId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static inline vec2 project(const vec3 &v, float far)
{
    float zScale = (far - v.z) / far;
    return vec2(v.x / zScale, v.y / zScale);
}

void OpenGLRenderer::render(Node *n)
{
    if (LayerNode *ln = Node::from<LayerNode>(n)) {
        const vec2 &p1 = m_matrixStack.top() * ln->position();
        const vec2 &p2 = m_matrixStack.top() * (ln->size() + ln->position());

        float data[] = { p1.x, p1.y, 0, 0,
                         p1.x, p2.y, 0, 1,
                         p2.x, p1.y, 1, 0,
                         p2.x, p2.y, 1, 1 };

        Layer *l = ln->layer();
        assert(l);
        assert(l->textureId());
        drawTextureQuad(data, l->textureId());

    } else if (RectangleNode *rn = Node::from<RectangleNode>(n)) {
        vec2 p1 = rn->position();
        vec2 p2 = (rn->size() + rn->position());

        if (m_currentProjectionDepth > 0 && m_currentTransformNode) {
            const mat4 &m = m_currentTransformNode->matrix();

            vec3 tl = m * vec3(p1);
            vec3 tr = m * vec3(p2.x, p1.y);
            vec3 bl = m * vec3(p1.x, p2.y);
            vec3 br = m * vec3(p2);

            vec2 ptl = project(tl, m_currentProjectionDepth);
            vec2 ptr = project(tr, m_currentProjectionDepth);
            vec2 pbl = project(bl, m_currentProjectionDepth);
            vec2 pbr = project(br, m_currentProjectionDepth);

            ptl = m_matrixStack.top() * ptl;
            ptr = m_matrixStack.top() * ptr;
            pbl = m_matrixStack.top() * pbl;
            pbr = m_matrixStack.top() * pbr;

            float data[] = { ptl.x, ptl.y,
                             pbl.x, pbl.y,
                             ptr.x, ptr.y,
                             pbr.x, pbr.y };
            drawColorQuad(data, rn->color());

        } else {
            p1 = m_matrixStack.top() * p1;
            p2 = m_matrixStack.top() * p2;
            float data[] = { p1.x, p1.y,
                             p1.x, p2.y,
                             p2.x, p1.y,
                             p2.x, p2.y };
            drawColorQuad(data, rn->color());

        }

    } else if (TransformNode *tn = Node::from<TransformNode>(n)) {
        if (tn->projectionDepth() > 0) {
            float oldProjDepth = m_currentProjectionDepth;
            TransformNode *oldTn = m_currentTransformNode;
            m_currentProjectionDepth = tn->projectionDepth();
            m_currentTransformNode = tn;

            for (auto it : n->children())
                render(it);

            m_currentTransformNode = oldTn;
            m_currentProjectionDepth = oldProjDepth;
        } else {
            m_matrixStack.push(m_matrixStack.top() * tn->matrix());
            for (auto it : n->children())
                render(it);
            m_matrixStack.pop();
        }
        return;

    }

    for (auto it : n->children())
        render(it);

}