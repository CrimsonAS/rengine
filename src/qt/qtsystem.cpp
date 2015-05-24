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

#include "../system.h"

#include <QGuiApplication>
#include <QWindow>
#include <QOpenGLContext>

using namespace rengine;

class QtSystem;
class QtSurface;
class QtWindow;
class QtOpenGLContext;

static int argc = 1;
static const char *argv[] = { "fake_rengine_app" };

class QtSystem : public System
{
public:
    QtSystem()
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
    QtWindow(QtSurface *s) :s(s) {
        resize(800, 480);
        setSurfaceType(QWindow::OpenGLSurface);
        create();
    }

    bool event(QEvent *e);
    void exposeEvent(QExposeEvent *e);
    void resizeEvent(QResizeEvent *e);

    QtSurface *s;
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
    vec2 size() const { return vec2(window.width(), window.height()); }
    void requestRender() { window.requestUpdate(); }

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



System *System::get()
{
    static QtSystem singleton;
    return &singleton;
}

void QtSystem::processEvents()
{
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
}

void QtSystem::run()
{
    app.exec();
}

Surface *QtSystem::createSurface(SurfaceInterface *iface)
{
    assert(iface);
    QtSurface *s = new QtSurface(iface);
    return s;
}

Renderer *QtSystem::createRenderer()
{
    OpenGLRenderer *r = new OpenGLRenderer();
    OpenGLContext *gl = createOpenGLContext();
    r->setOpenGLContext(gl);
    return r;
}

OpenGLContext *QtSystem::createOpenGLContext()
{
    return new QtOpenGLContext();
}

bool QtWindow::event(QEvent *e)
{
    if (e->type() == QEvent::UpdateRequest) {
        s->iface->onRender();
        return true;
    }
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


