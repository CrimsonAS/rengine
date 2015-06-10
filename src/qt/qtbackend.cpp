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

#include <QGuiApplication>
#include <QWindow>
#include <QOpenGLContext>

#include "rengine.h"

#if QT_VERSION >= 0x050500
#define QWINDOW_HAS_REQUEST_UPDATE
#endif

RENGINE_USE_NAMESPACE;

class QtBackend;
class QtSurface;
class QtWindow;
class QtOpenGLContext;

static int argc = 1;
static const char *argv[] = { "fake_rengine_app" };

class QtBackend : public Backend
{
public:
    QtBackend()
        : app(argc, (char **) argv)
    {
    }

    void run();
    void processEvents();
    Surface *createSurface(SurfaceInterface *iface);
    Renderer *createRenderer();
    OpenGLContext *createOpenGLContext();

    QGuiApplication app;
};

class QtWindow : public QWindow
{
public:
    QtWindow(QtSurface *s)
    : s(s)
#ifndef QWINDOW_HAS_REQUEST_UPDATE
    , updateTimer(0)
#endif
    {
        resize(800, 480);
        setSurfaceType(QWindow::OpenGLSurface);
        create();
    }

    bool event(QEvent *e);
    void exposeEvent(QExposeEvent *e);
    void resizeEvent(QResizeEvent *e);

    QtSurface *s;

#ifndef QWINDOW_HAS_REQUEST_UPDATE
    void requestUpdate() {
        if (updateTimer == 0)
            updateTimer = startTimer(5);
    }
    int updateTimer;
#endif
};

class QtSurface : public Surface
{
public:
    QtSurface(SurfaceInterface *iface)
    : window(this)
    , iface(iface)
    {
        setSurfaceToInterface(iface);
    }

    void show() { window.show(); }
    void hide() { window.hide(); }
    vec2 size() const {
        int dpr = window.devicePixelRatio();
        return vec2(window.width() * dpr, window.height() * dpr);
    }
    void requestRender() {
        window.requestUpdate();
    }

    QtWindow window;
    SurfaceInterface *iface;
};



class QtOpenGLContext : public OpenGLContext
{
public:
    QtOpenGLContext()
    {
        context.create();
    }

    bool makeCurrent(Surface *s) {
        return context.makeCurrent(&static_cast<QtSurface *>(s)->window);
    }

    bool swapBuffers(Surface *s) {
        context.swapBuffers(&static_cast<QtSurface *>(s)->window);
        return true;
    }

    QOpenGLContext context;
};



Backend *Backend::get()
{
    static QtBackend *singleton = new QtBackend();
    return singleton;
}

void QtBackend::processEvents()
{
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
}

void QtBackend::run()
{
    app.exec();
}

Surface *QtBackend::createSurface(SurfaceInterface *iface)
{
    assert(iface);
    QtSurface *s = new QtSurface(iface);
    return s;
}

Renderer *QtBackend::createRenderer()
{
    OpenGLRenderer *r = new OpenGLRenderer();
    OpenGLContext *gl = createOpenGLContext();
    r->setOpenGLContext(gl);
    return r;
}

OpenGLContext *QtBackend::createOpenGLContext()
{
    return new QtOpenGLContext();
}

bool QtWindow::event(QEvent *e)
{
#ifdef QWINDOW_HAS_REQUEST_UPDATE
    if (e->type() == QEvent::UpdateRequest) {
        s->iface->onRender();
        return true;
    }
#else
    if (e->type() == QEvent::Timer) {
        killTimer(updateTimer);
        updateTimer = 0;
        s->iface->onRender();
    }
#endif
    return QWindow::event(e);
}

void QtWindow::exposeEvent(QExposeEvent *e)
{
    if (isExposed())
        s->iface->onRender();
}

void QtWindow::resizeEvent(QResizeEvent *e)
{
    s->iface->onSizeChange(vec2(width(), height()));
}


