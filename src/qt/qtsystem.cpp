#include "../system.h"

#include <QGuiApplication>
#include <QWindow>
#include <QOpenGLContext>

using namespace rengine;

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
    Surface *createSurface();
    Renderer *createRenderer();
    OpenGLContext *createOpenGLContext();

    QGuiApplication app;
};



class QtSurface : public Surface
{
public:
    QtSurface() {
        window.resize(800, 480);
        window.setSurfaceType(QWindow::OpenGLSurface);
        window.create();
    }

    void show() {
        window.show();
    }

    void hide() {
        window.hide();
    }

    vec2 size() const {
        return vec2(window.width(), window.height());
    }

    QWindow window;
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

Surface *QtSystem::createSurface()
{
    return new QtSurface();
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
