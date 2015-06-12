/*
    Copyright (c) 2015, Robin Burchell <robin.burchell@viroteck.net>
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

#include <SDL.h>

#include "rengine.h"

RENGINE_USE_NAMESPACE;
using namespace std;

class SdlBackend;
class SdlSurface;
class SdlWindow;

static void sdldie(const char *msg)
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}

class SdlBackend : public Backend
{
public:
    SdlBackend()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            sdldie("Unable to initialize SDL");
#ifdef RENGINE_LOG_INFO
        cout << "SdlBackend: created..." << endl;
#endif
    }

    void run();
    void processEvents();
    Surface *createSurface(SurfaceInterface *iface);
    Renderer *createRenderer(Surface *surface);
};

class SdlWindow
{
public:
    SdlWindow(SdlSurface *s)
        : s(s)
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        mainwindow = SDL_CreateWindow("rengine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      800, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        context = SDL_GL_CreateContext(mainwindow);
        SDL_GL_SetSwapInterval(1);

    }

    SdlSurface *s;
    SDL_Window *mainwindow;
    SDL_GLContext context;
};

class SdlSurface : public Surface
{
public:
    SdlSurface(SurfaceInterface *iface)
    : window(this)
    , iface(iface)
    {
        setSurfaceToInterface(iface);
#ifdef RENGINE_LOG_INFO
        cout << "SdlBackend::Surface created with interface=" << iface << endl;
#endif
        requestRender();
    }

    bool makeCurrent() {
        static bool warned = false;
        if (!warned) {
            warned = true;
            cerr << "makeCurrent: stub" << endl;
        }
        return true;
    }

    bool swapBuffers() {
        SDL_GL_SwapWindow(window.mainwindow);
        return true;
    }

    void show() {
        SDL_ShowWindow(window.mainwindow);
    }

    void hide() {
        SDL_HideWindow(window.mainwindow);
    }

    vec2 size() const {
        int w;
        int h;
        SDL_GetWindowSize(window.mainwindow, &w, &h);
        return vec2(w, h);
    }

    void requestRender() {
        // we can't trigger the render synchronously. we need to give a chance
        // to process input, animations, whatever -- so push an event onto the
        // queue and we'll get back to this later.
        SDL_Event event;

        SDL_UserEvent renderev;
        renderev.type = SDL_USEREVENT;
        renderev.code = 0;
        renderev.data1 = this;
        renderev.data2 = NULL;

        event.type = SDL_USEREVENT;
        event.user = renderev;

        SDL_PushEvent(&event);
    }

    SdlWindow window;
    SurfaceInterface *iface;
};

Backend *Backend::get()
{
    static SdlBackend *singleton = new SdlBackend();
    return singleton;
}

void SdlBackend::processEvents()
{
    cerr << "stub: processEvents" << endl;
}

void SdlBackend::run()
{
#ifdef RENGINE_LOG_INFO
    cout << "SdlBackend: starting eventloop..." << endl;
#endif

    bool done = false;
    SDL_Event event;

    while (!done && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_USEREVENT: {
                // process the asynchronous render request
                SdlSurface *surface = static_cast<SdlSurface *>(event.user.data1);
                surface->iface->onRender();
                break;
            }
            case SDL_QUIT: {
                done = true;
                break;
            }
            default: {
                cerr << "unknown event.type " << event.type << endl;
            }
        }
    }

#ifdef RENGINE_LOG_INFO
    cout << "SdlBackend: exited eventloop..." << endl;
#endif
}

Surface *SdlBackend::createSurface(SurfaceInterface *iface)
{
    assert(iface);
    SdlSurface *s = new SdlSurface(iface);
    return s;
}

Renderer *SdlBackend::createRenderer(Surface *surface)
{
    assert(surface);
    OpenGLRenderer *r = new OpenGLRenderer();
    r->setTargetSurface(surface);
    return r;
}

