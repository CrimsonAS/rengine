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

    void setSceneRoot(Node *root) { m_sceneRoot = root; }

    void setTargetSurface(Surface *surface) { m_surface = surface; }

    void initialize();

    bool render();

private:
    void render(Node *n);

    Node *m_sceneRoot;
    Surface *m_surface;
    OpenGLContext *m_gl;

    struct LayerProgram : public OpenGLShaderProgram {
        int matrix;
    } prog_layer;
    struct SolidProgram : public OpenGLShaderProgram {
        int matrix;
        int color;
    } prog_solid;

    std::stack<mat4> m_matrixStack;
};

RENGINE_END_NAMESPACE
