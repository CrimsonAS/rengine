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

#include <QGuiApplication>
#include <QWindow>
#include <QOpenGLContext>

#include "rengine.h"

#if QT_VERSION >= 0x050500
#define QWINDOW_HAS_REQUEST_UPDATE
#endif

RENGINE_USE_NAMESPACE;
using namespace std;

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
        : app(argc, (char **) argv), exited(false)
    {
#ifdef RENGINE_LOG_INFO
        cout << "QtBackend: created..." << endl;
#endif
    }

    void quit() override { cout << "quitting..." << endl; exited = true; app.quit(); }
    void run();
    void processEvents();
    Surface *createSurface(SurfaceInterface *iface);
    Renderer *createRenderer(Surface *surface);

    QGuiApplication app;

    bool exited;
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
#ifdef RENGINE_LOG_INFO
        cout << "QtBackend::Surface created with interface=" << iface << endl;
#endif
        QSurfaceFormat format;
        format.setSamples(4); // ### TODO: make this settable a bit more conveniently...

        window.setFormat(format);
        window.setSurfaceType(QSurface::OpenGLSurface);
        window.resize(800, 480);
        window.create();

        context.setFormat(format);
        context.create();
    }

    bool makeCurrent() {
        if (!context.makeCurrent(&window)) {
            cout << "QtSurface::makeCurrent: failed..." << endl;
            return false;
        }
        return true;
    }

    bool swapBuffers() {
        context.swapBuffers(&window);
        return context.isValid();
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

    QOpenGLContext context;
    QtWindow window;
    SurfaceInterface *iface;
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
#ifdef RENGINE_LOG_INFO
    cout << "QtBackend: starting eventloop..." << endl;
    if (exited)
        cout << " -> quit() already called, not starting eventloop.." << endl;
#endif
    if (!exited)
        app.exec();
#ifdef RENGINE_LOG_INFO
    cout << "QtBackend: exited eventloop..." << endl;
#endif
}

Surface *QtBackend::createSurface(SurfaceInterface *iface)
{
    assert(iface);
    QtSurface *s = new QtSurface(iface);
    return s;
}

Renderer *QtBackend::createRenderer(Surface *surface)
{
    assert(surface);
    assert(&static_cast<QtSurface *>(surface)->context == QOpenGLContext::currentContext());
    OpenGLRenderer *r = new OpenGLRenderer();
    r->setTargetSurface(surface);
    return r;
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
    if (isExposed() && isVisible())
        s->iface->onRender();
}

void QtWindow::resizeEvent(QResizeEvent *e)
{
    s->iface->onSizeChange(vec2(width(), height()));
}


