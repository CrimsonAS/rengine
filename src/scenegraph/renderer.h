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

#include "node.h"
#include "layer.h"

namespace rengine {

class Renderer
{
public:
    Renderer()
    : m_fillColor(0, 0, 0, 1)
    {
    }

    virtual ~Renderer() {}

    /*!
        Creates a layer from image data which is compatible with this
        renderer. The image data is 32-bit RGBA or RGBx, tightly packed.
     */
    virtual Layer *createLayerFromImageData(const vec2 &size, Layer::Format format, void *data) = 0;

    /*!
        Sets \a root as the scene root for this renderer
     */
    virtual void setSceneRoot(Node *root) = 0;

    virtual void setTargetSurface(Surface *surface) = 0;

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

    void setFillColor(const vec4 &c) { m_fillColor = c; }
    const vec4 &fillColor() const { return m_fillColor; }

private:
    vec4 m_fillColor;
};

}