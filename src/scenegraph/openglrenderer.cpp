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

#include "../rengine.h"

#include <stdio.h>

using namespace rengine;
using namespace std;

const char *vsh_es_solid =
RENGINE_GLSL_HEADER
"\
attribute highp vec2 aV;                        \n\
uniform highp mat4 m;                           \n\
void main() {                                   \n\
    gl_Position = m * vec4(aV, 0, 1);           \n\
}                                               \n\
";

const char *fsh_es_solid =
RENGINE_GLSL_HEADER
"\
uniform lowp vec4 color;                        \n\
void main() {                                   \n\
    gl_FragColor = color;                       \n\
}                                               \n\
";


const char *vsh_es_layer =
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

const char *fsh_es_layer =
RENGINE_GLSL_HEADER
"\
uniform lowp sampler2D t;                       \n\
varying highp vec2 vT;                          \n\
void main() {                                   \n\
    gl_FragColor = texture2D(t, vT);            \n\
}                                               \n\
";

OpenGLRenderer::OpenGLRenderer()
    : m_sceneRoot(0)
{

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
    m_gl->makeCurrent(m_surface);

    { // Default layer shader
        vector<const char *> attrs;
        attrs.push_back("aV");
        attrs.push_back("aT");
        cout << "building layer shader: " << attrs.size() << endl;
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
    m_gl->makeCurrent(m_surface);

    vec2 surfaceSize = m_surface->size();
    glViewport(0, 0, surfaceSize.x, surfaceSize.y);

    vec4 c = fillColor();
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glUseProgram(prog_layer.id());
    mat4 proj = mat4::translate2D(-1.0, 1.0)
                * mat4::scale2D(2.0f / surfaceSize.x, -2.0f / surfaceSize.y);

    assert(m_matrixStack.empty());
    m_matrixStack.push(proj);

    render(m_sceneRoot);

    m_matrixStack.pop();
    assert(m_matrixStack.empty());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glUseProgram(0);

    m_gl->swapBuffers(m_surface);

    return true;
}

void OpenGLRenderer::render(Node *n)
{
    if (LayerNode *ln = Node::from<LayerNode>(n)) {
        vec2 s = ln->size();
        float data[] = {   0,   0, 0, 0,
                           0, s.y, 0, 1,
                         s.x,   0, 1, 0,
                         s.x, s.y, 1, 1 };
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &data[0]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &data[2]);

        Layer *l = ln->layer();
        assert(l);
        assert(l->textureId());

        glUniformMatrix4fv(prog_layer.matrix, 1, true, m_matrixStack.top().m);
        glBindTexture(GL_TEXTURE_2D, l->textureId());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    } else if (TransformNode *tn = Node::from<TransformNode>(n)) {
        m_matrixStack.push(m_matrixStack.top() * tn->matrix());
    }

    for (auto it : n->children())
        render(it);

    if (Node::from<TransformNode>(n))
        m_matrixStack.pop();

}