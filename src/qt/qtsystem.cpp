#include "../system.h"

#include <QGuiApplication>
#include <QWindow>

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
    Window *createWindow();
    Renderer *createRenderer(Window *window);

    QGuiApplication app;
};



class QtWindow : public Window
{
public:
    QtWindow() {
        window.resize(800, 480);
    }

    void show() {
        printf("showing qt window...\n");
        window.show();
    }

    void hide() {
        printf("hiding qt window...\n");
        window.hide();
    }

    vec2 size() const {
        return vec2(window.width(), window.height());
    }

    QWindow window;
};



System *System::get()
{
    static QtSystem singleton;
    return &singleton;
}

void QtSystem::run()
{
    app.exec();
}

Window *QtSystem::createWindow()
{
    return new QtWindow();
}

Renderer *QtSystem::createRenderer(Window *window)
{
    return new OpenGLRenderer();
}
