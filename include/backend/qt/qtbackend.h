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

#define QT_NO_KEYWORDS

#include <QGuiApplication>
#include <QWindow>
#include <QOpenGLContext>
#include <QTimer>

#if QT_VERSION >= 0x050500
#define QWINDOW_HAS_REQUEST_UPDATE
#endif

RENGINE_BEGIN_NAMESPACE

class QtBackend;
class QtSurface;
class QtWindow;
class QtOpenGLContext;

class QtBackend : public Backend
{
    static int &hack_argc() { static int i = 1; return i; }
    static char **hack_argv() { static const char *v[] = { "rengine_app" }; return (char **) v; }
public:

    QtBackend()
        : app(hack_argc(), hack_argv()), exited(false)
    {
#ifdef RENGINE_LOG_INFO
        std::cout << "QtBackend: created..." << std::endl;
#endif
    }

    void quit() override { exited = true; app.quit(); }
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
        logd << "using timer" << std::endl;
        if (updateTimer == 0)
            updateTimer = startTimer(5);
    }
    int updateTimer;
#else
    void requestUpdate() {
        logd << "using QWindow::requestUpdate()" << std::endl;
        QWindow::requestUpdate();
    }
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
        logi << "QtBackend::Surface created with interface=" << iface << std::endl;
        QSurfaceFormat format;
        // format.setSamples(4); // ### TODO: make this settable a bit more conveniently...

        window.setFormat(format);
        window.setSurfaceType(QSurface::OpenGLSurface);
        window.resize(800, 480);
        window.create();

        context.setFormat(format);
        context.create();
    }

    bool makeCurrent() {
        if (!context.makeCurrent(&window)) {
            logw << "failed..." << std::endl;
            return false;
        }
        return true;
    }

    bool swapBuffers() {
        if (context.swapBuffers(&window)) {
            logw << "failed.." << std::end;
        }
        return context.isValid();
    }

    void show() { QTimer::singleShot(0, &window, SLOT(show())); }
    void hide() { QTimer::singleShot(0, &window, SLOT(hide())); }
    vec2 size() const {
        int dpr = window.devicePixelRatio();
        return vec2(window.width() * dpr, window.height() * dpr);
    }
    void requestRender() {
        logd << std::endl;
        window.requestUpdate();
    }

    QOpenGLContext context;
    QtWindow window;
    SurfaceInterface *iface;
};

inline void QtBackend::processEvents()
{
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
}

inline void QtBackend::run()
{
    logd << "starting eventloop..." << std::endl;
    if (exited)
        logd << "quit() already called, not starting eventloop.." << std::endl;
    if (!exited)
        app.exec();
    logd << "exited event loop" << << std::endl;
}

inline Surface *QtBackend::createSurface(SurfaceInterface *iface)
{
    logd << __PRETTY_FUNCTION__ << iface << std::endl;
    assert(iface);
    QtSurface *s = new QtSurface(iface);
    return s;
}

inline Renderer *QtBackend::createRenderer(Surface *surface)
{
    logd << __PRETTY_FUNCTION__ << surface << std::endl;
    assert(surface);
    assert(&static_cast<QtSurface *>(surface)->context == QOpenGLContext::currentContext());
    OpenGLRenderer *r = new OpenGLRenderer();
    r->setTargetSurface(surface);
    return r;
}

inline bool QtWindow::event(QEvent *e)
{
#ifdef QWINDOW_HAS_REQUEST_UPDATE
    if (e->type() == QEvent::UpdateRequest) {
        logd << "invoking onRender" << std::endl;
        s->iface->onRender();
        return true;
    }
#else
    if (e->type() == QEvent::Timer) {
        logd << "invoking onRender" << std::endl;
        killTimer(updateTimer);
        updateTimer = 0;
        s->iface->onRender();
    }
#endif
    return QWindow::event(e);
}

inline void QtWindow::exposeEvent(QExposeEvent *e)
{
    logd << std::endl;
    if (isExposed() && isVisible())
        s->iface->onRender();
}

inline void QtWindow::resizeEvent(QResizeEvent *e)
{
    logd << e->width() << "x" << e->height() << std::endl;
    s->iface->onSizeChange(vec2(width(), height()));
}

RENGINE_END_NAMESPACE

#define RENGINE_BACKEND_DEFINE                         \
    Backend *Backend::get() {                          \
        static QtBackend *singleton = new QtBackend(); \
        return singleton;                              \
    }                                                  \


