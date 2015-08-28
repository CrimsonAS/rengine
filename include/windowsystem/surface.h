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

class Surface
{
public:
    virtual ~Surface() {}

    /*!
        Call to hide this surface..

        TODO: Is this really needed?
     */
    virtual void hide() = 0;

    /*!
        Call to show this surface..
     */
    virtual void show() = 0;

    /*!
        Called by the application to start a new frame. Makes the rendering
        context current.

        Returns true if all is well; false if it failed for whatever reason..
     */
    virtual bool makeCurrent() = 0;

    /*!
        Called by the application it is done rendering to push the current
        content to screen.

        Returns true if all went wll; false if it failed for whatever reason..

     */
    virtual bool swapBuffers() = 0;

    /*!
        Returns the size of this surface. When the surface size changes,
        the client is notified via SurfaceInterface::onSizeChange()

        TODO: should really be an ivec2, but I don't have that yet :p
     */
    virtual vec2 size() const = 0;

    virtual void requestRender() = 0;

protected:
    void setSurfaceToInterface(SurfaceInterface *iface);
};

class SurfaceInterface
{
public:
    SurfaceInterface() : m_surface(0) { }
    virtual ~SurfaceInterface() { }

    /*!
        Reimplement this function get notified when it is time to
        render the surface
     */
    virtual void onRender() { };

    /*!
        Reimplement this function to get notified when the surface's
        size has changed.
     */
    virtual void onSizeChange(const vec2 &size) { }

    Surface *surface() { return m_surface; }
    const Surface *surface() const { return m_surface; }

    virtual void dispatchEvent(Event *e) { }

private:
    friend class Surface;

    Surface *m_surface;
};

inline void Surface::setSurfaceToInterface(SurfaceInterface *iface) {
    iface->m_surface = this;
}

RENGINE_END_NAMESPACE
