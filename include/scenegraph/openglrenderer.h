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

#pragma once

#include <stack>

RENGINE_BEGIN_NAMESPACE

class OpenGLRenderer : public Renderer
{
public:
    OpenGLRenderer();
    Layer *createLayerFromImageData(const vec2 &size, Layer::Format format, void *data);
    void setOpenGLContext(OpenGLContext *gl) { m_gl = gl; }
    void initialize();
    bool render();

private:
    struct NodeToRender {
        Node *node;
        float vertices[8];
        float z;
        bool operator<(const NodeToRender &o) const { return z < o.z; }
    };

    struct RenderState {
        RenderState()
            : farPlane(0)
        {
        }

        std::stack<mat4> matrices;

        void push(const mat4 &m) { matrices.push(matrices.top() * m); }
        void pop() { matrices.pop(); }

        std::vector<NodeToRender> nodes;

        float farPlane;
    };

    void prepass(Node *n);
    void render(Node *n);
    void drawColorQuad(const float *v, const vec4 &color);
    void drawTextureQuad(const float *v, GLuint texId);
    void activateShader(const OpenGLShaderProgram *shader);
    void projectQuad(const vec2 &a, const vec2 &b, float *v);
    void gatherNodes3D(Node *n);
    void render3D();
    RenderState *state();

    OpenGLContext *m_gl;

    struct LayerProgram : public OpenGLShaderProgram {
        int matrix;
    } prog_layer;
    struct SolidProgram : public OpenGLShaderProgram {
        int matrix;
        int color;
    } prog_solid;

    std::vector<RenderState> m_states;

    OpenGLShaderProgram *m_activeShader;

    unsigned m_numOpacityNodes;
    unsigned m_numLayerNodes;
    unsigned m_numRectangleNodes;
    unsigned m_numTransformNodes;
    unsigned m_numTransformNodesWith3d;
};

inline OpenGLRenderer::RenderState *OpenGLRenderer::state() { return &m_states.back(); }

inline void OpenGLRenderer::projectQuad(const vec2 &a, const vec2 &b, float *v)
{
    const mat4 &M3D = state()->matrices.top();
    const mat4 &P = m_states.at(m_states.size() - 2).matrices.top();
    const float farPlane = state()->farPlane;

    // The steps involved in each line is as follows.:
    // pt_3d = matrix3D * pt                 // apply the 3D transform
    // pt_proj = pt_3d.project2D()           // project it to 2D based on current farPlane
    // pt_screen = parent_matrix * pt_proj   // Put the output of our local 3D into the scene world coordinate system

    // Output the results direclty into the v array
    vec2 *vv = (vec2 *) v;
    vv[0] = P * ((M3D * vec3(a))       .project2D(farPlane));    // top left
    vv[1] = P * ((M3D * vec3(a.x, b.y)).project2D(farPlane));    // bottom left
    vv[2] = P * ((M3D * vec3(b.x, a.y)).project2D(farPlane));    // top right
    vv[3] = P * ((M3D * vec3(b))       .project2D(farPlane));    // bottom right
}


RENGINE_END_NAMESPACE
