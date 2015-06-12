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

RENGINE_BEGIN_NAMESPACE

class OpenGLRenderer : public Renderer
{
public:
    struct Element {
        Node *node;
        rect2d bounds;          // only valid when 'layered' is set
        float z;                // only valid when 'projection' is set
        unsigned range;         // offset into vbo for draw nodes, size of 'batch' for projection/layered
        bool projection : 1;    // 3d subtree
        bool layered : 1;       // as in flattened subtree

        bool operator<(const Element &e) const { assert(projection); return z < e.z; }
    };

    OpenGLRenderer();
    ~OpenGLRenderer();

    Layer *createLayerFromImageData(const vec2 &size, Layer::Format format, void *data);
    void initialize();
    bool render() override;

    void prepass(Node *n);
    void build(Node *n);
    void drawColorQuad(unsigned bufferOffset, const vec4 &color);
    void drawTextureQuad(unsigned bufferOffset, GLuint texId);
    void activateShader(const OpenGLShaderProgram *shader);
    void projectQuad(const vec2 &a, const vec2 &b, vec2 *v);
    void render(unsigned first, unsigned last);

    struct LayerProgram : public OpenGLShaderProgram {
        int matrix;
    } prog_layer;
    struct SolidProgram : public OpenGLShaderProgram {
        int matrix;
        int color;
    } prog_solid;

    unsigned m_numLayeredNodes;
    unsigned m_numTextureNodes;
    unsigned m_numRectangleNodes;
    unsigned m_numTransformNodes;
    unsigned m_numTransformNodesWith3d;

    unsigned m_vertexIndex;
    unsigned m_elementIndex;
    vec2 *m_vertices;
    Element *m_elements;
    mat4 m_m2d;    // world matrix
    mat4 m_m3d;    // projection matrix
    float m_farPlane;

    OpenGLShaderProgram *m_activeShader;
    GLuint m_texCoordBuffer;
    GLuint m_vertexBuffer;

    bool m_render3d : 1;
    bool m_layered : 1;

};

inline void OpenGLRenderer::projectQuad(const vec2 &a, const vec2 &b, vec2 *v)
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


RENGINE_END_NAMESPACE
