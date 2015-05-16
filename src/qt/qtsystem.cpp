#include "../system.h"

using namespace rengine;



class QtSystem : public System
{
public:
    void run();
    Window *createWindow();
    Renderer *createRenderer(Window *window);
};



class QtWindow : public Window
{
public:
    void show() {
        printf("showing qt window...\n");
        // window.show();
    }

    void hide() {
        printf("hiding qt window...\n");
    }

    vec2 size() const {
        return vec2(800, 600);
    }
    // QWindow window;
};



System *System::get()
{
    static QtSystem singleton;
    return &singleton;
}

void QtSystem::run()
{
    // app.exec();
}

Window *QtSystem::createWindow()
{
    return new QtWindow();
}

Renderer *QtSystem::createRenderer(Window *window)
{
    printf("returning qtsystem's renderer...\n");
    return 0;
}
