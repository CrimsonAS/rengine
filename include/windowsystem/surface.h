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

/*!

    This class is implemented in the backend to support the implementation of
    a surface on that backend.
 */
class SurfaceBackendImpl
{
public:
    virtual ~SurfaceBackendImpl() { }

    /*!
        Implement in the backend to hide the surface.
     */
    virtual void hide() = 0;

    /*!
        Implement in the backend to show the surface.
     */
    virtual void show() = 0;

    /*!
        Implement in the backend to begin rendering. The backend can assume that
        this is called once for each render cycle.
     */
    virtual bool beginRender() = 0;

    /*!
        Implement in the backend to finish rendering and present it to screen. This
        is the equivalent of swabBuffers in the OpenGL.
     */
    virtual bool commitRender() = 0;

    /*!
        Implement in the backend to report the size of a surface to the application
     */
    virtual vec2 size() const = 0;

    /*!
        Implement in the backend to respond to requests by the application to change
        the surface's size
     */
    virtual void requestSize(vec2 size) = 0;

    /*!
        Implement in the backend to respond to requests for Surface::onRender() to
        be called.
     */
    virtual void requestRender() = 0;

    /*!
        Implement in the backend to create a renderer compatible with this surface
     */
    virtual Renderer *createRenderer() = 0;

};

class Surface
{
public:
    Surface()
    {
        m_impl = Backend::get()->createSurface(this);
    }

    virtual ~Surface()
    {
        Backend::get()->destroySurface(this, m_impl);
    }

    void hide() { m_impl->hide(); }

    void show() { m_impl->show(); }

    bool beginRender() { return m_impl->beginRender(); }

    bool commitRender() { return m_impl->commitRender(); }

    vec2 size() const { return m_impl->size(); }

    void requestSize(vec2 size) { m_impl->requestSize(size); }

    void requestRender() { m_impl->requestRender(); }

    Renderer *createRenderer() { return m_impl->createRenderer(); }

    /*!
        Reimplement this function get notified when it is time to
        render the surface
     */
    virtual void onRender() { };

    /*!

        Called by the backend when an event occurs. The surface interface should
        implement this method to deliver the event to the right place inside
        the application.

     */
    virtual void onEvent(Event *) { }

private:
    SurfaceBackendImpl *m_impl;
};


RENGINE_END_NAMESPACE
