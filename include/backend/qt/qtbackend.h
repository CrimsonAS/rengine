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
#include <QResizeEvent>

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
        logd << std::endl;
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
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

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

    void sendPointerEvent(QMouseEvent *e, Event::Type type);
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
        format.setRedBufferSize(8);
        format.setBlueBufferSize(8);
        format.setGreenBufferSize(8);
        format.setAlphaBufferSize(8);
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
        logd << "swapping" << std::endl;
        context.swapBuffers(&window);
        logd << "swapping done" << std::endl;
        if (!context.isValid()) {
            logw << "context is no longer valid" << std::endl;
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
    logd << "exited event loop" << std::endl;
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

inline void QtWindow::sendPointerEvent(QMouseEvent *e, Event::Type type)
{
    PointerEvent pe(type);
    pe.initialize(vec2(e->x(), e->y()) * s->window.devicePixelRatio());
    s->iface->onEvent(&pe);
}

inline bool QtWindow::event(QEvent *e)
{
#ifdef QWINDOW_HAS_REQUEST_UPDATE
    if (e->type() == QEvent::UpdateRequest) {
        logd << "invoking onRender (update from updateRequest)" << std::endl;
        s->iface->onRender();
        logd << "onRender completed" << std::endl;
        return true;
    }
#else
    if (e->type() == QEvent::Timer) {
        logd << "updateRequest from timer triggered" << std::endl;
        killTimer(updateTimer);
        updateTimer = 0;
        logd << "invoking onRender" << std::endl;
        s->iface->onRender();
        logd << "onRender completed" << std::endl;
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
    logd << e->size().width() << "x" << e->size().height() << std::endl;
    s->iface->onSizeChange(vec2(width(), height()));
}

void QtWindow::mousePressEvent(QMouseEvent *e)
{
    sendPointerEvent(e, Event::PointerDown);
}

void QtWindow::mouseReleaseEvent(QMouseEvent *e)
{
    sendPointerEvent(e, Event::PointerUp);
}

void QtWindow::mouseMoveEvent(QMouseEvent *e)
{
    sendPointerEvent(e, Event::PointerMove);
}

RENGINE_END_NAMESPACE

#define RENGINE_BACKEND_DEFINE                         \
    Backend *Backend::get() {                          \
        static QtBackend *singleton = new QtBackend(); \
        return singleton;                              \
    }                                                  \


