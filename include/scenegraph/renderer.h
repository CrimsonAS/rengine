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

RENGINE_BEGIN_NAMESPACE

class Renderer
{
public:
    Renderer()
        : m_sceneRoot(0)
        , m_surface(0)
        , m_fillColor(0, 0, 0, 1)
    {
    }

    virtual ~Renderer() {}

    /*!
        Creates a layer from image data which is compatible with this
        renderer. The image data is 32-bit RGBA or RGBx, tightly packed.
     */
    virtual Layer *createLayerFromImageData(const vec2 &size, Layer::Format format, void *data) = 0;

    Node *sceneRoot() const { return m_sceneRoot; }
    void setSceneRoot(Node *root) { m_sceneRoot = root; }

    Surface *targetSurface() const { return m_surface; }
    void setTargetSurface(Surface *surface) { m_surface = surface; }

    /*!
        The application should call this before the very first render pass.

        Once this function is called, the scene graph can be built and
        put to the renderer.
     */
    virtual void initialize() = 0;

    /*!
        Renders the scene graph specified using setSceneRoot().

        Returns true if successful; false if rendering failed...
     */
    virtual bool render() = 0;

    /*!
        Read back pixels into \a bytes.

        The pixels will be 32 bit RGBA and tightly packed into \a width and \a
        height.
     */
    virtual bool readPixels(int x, int y, int width, int height, unsigned char *bytes) { return false; }

    /*!
        Called after the frame has been swapped. The renderer can use this
        to perform post-frame cleanup, for instance...
     */
    virtual void frameSwapped() { }

    void setFillColor(const vec4 &c) { m_fillColor = c; }
    const vec4 &fillColor() const { return m_fillColor; }

private:
    Node *m_sceneRoot;
    Surface *m_surface;
    vec4 m_fillColor;
};

RENGINE_END_NAMESPACE