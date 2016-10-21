/*
    Copyright (c) 2015, Robin Burchell <robin.burchell@viroteck.net>
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

#include <SDL.h>

RENGINE_BEGIN_NAMESPACE

class SdlBackend;
class SdlSurface;
class SdlWindow;

inline void sdlbackend_die(const char *msg)
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
            sdlbackend_die("Unable to initialize SDL");
#ifdef RENGINE_LOG_INFO
        std::cout << "SdlBackend: created..." << std::endl;
#endif
    }

    ~SdlBackend()
    {
        SDL_Quit();
    }

    void processEvents() override;
    Surface *createSurface(SurfaceInterface *iface) override;
    Renderer *createRenderer(Surface *surface) override;

    void sendPointerEvent(SDL_Event *e, Event::Type type);

    SdlSurface *findSurface(unsigned id);

private:
    std::vector<SdlSurface *> m_surfaces;
};

class SdlWindow
{
public:
    SdlWindow(SdlSurface *s)
        : s(s)
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

        mainwindow = SDL_CreateWindow("rengine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      800, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI );

        context = SDL_GL_CreateContext(mainwindow);
        SDL_GL_SetSwapInterval(1);
    }

    unsigned id() const { return SDL_GetWindowID(mainwindow); }

    unsigned devicePixelRatio() const {
        int dw, dh, ww, wh;
        SDL_GetWindowSize(mainwindow, &ww, &wh);
        SDL_GL_GetDrawableSize(mainwindow, &dw, &dh);
        return dw / ww;
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
        std::cout << "SdlBackend::Surface created with interface=" << iface << std::endl;
#endif
        requestRender();
    }

    bool makeCurrent() {
        static bool warned = false;
        if (!warned) {
            warned = true;
            std::cerr << "makeCurrent: stub" << std::endl;
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
        SDL_GL_GetDrawableSize(window.mainwindow, &w, &h);
        return vec2(w, h);
    }

    void render() {
        // reset this before onRender so we don't prevent onRender from
        // scheduling another one..
        m_renderRequested = false;
        iface->onRender();
    }

    void requestRender() {
        if (m_renderRequested)
            return;
        m_renderRequested = true;
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

    bool m_renderRequested = false;

    SdlWindow window;
    SurfaceInterface *iface;
};

inline SdlSurface *SdlBackend::findSurface(unsigned id)
{
    for (auto surface : m_surfaces)
        if (surface->window.id() == id)
            return surface;
    return 0;
}

inline void SdlBackend::processEvents()
{
    SDL_Event event;
    int evt = SDL_PollEvent(nullptr);

    // This odd-looking construct ensures we do not process events that are
    // pushed onto the queue after we start processing, so as to not starve the
    // main loop.
    while (evt-- > 0) {
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_USEREVENT: {
                SdlSurface *surface = static_cast<SdlSurface *>(event.user.data1);
                // process the asynchronous render request
                surface->render();
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                sendPointerEvent(&event, Event::PointerDown);
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                sendPointerEvent(&event, Event::PointerUp);
                break;
            }
            case SDL_MOUSEMOTION: {
                sendPointerEvent(&event, Event::PointerMove);
                break;
            }
            case SDL_QUIT: {
                m_running = false;
                break;
            }
            default: {
                // std::cerr << "unknown event.type " << event.type << std::endl;
            }
        }
    }
}

inline void SdlBackend::sendPointerEvent(SDL_Event *sdlEvent, Event::Type type)
{
    PointerEvent pe(type);
    SdlSurface *surface = findSurface(sdlEvent->button.windowID);
    pe.initialize(vec2(sdlEvent->button.x, sdlEvent->button.y) * surface->window.devicePixelRatio());
    assert(surface);
    surface->iface->onEvent(&pe);
}


inline Surface *SdlBackend::createSurface(SurfaceInterface *iface)
{
    assert(iface);
    SdlSurface *s = new SdlSurface(iface);
    m_surfaces.push_back(s);
    return s;
}

inline Renderer *SdlBackend::createRenderer(Surface *surface)
{
    assert(surface);
    OpenGLRenderer *r = new OpenGLRenderer();
    r->setTargetSurface(surface);
    return r;
}

#define RENGINE_BACKEND_DEFINE                                             \
    rengine::Backend *rengine::Backend::get() {                            \
        static rengine::SdlBackend *singleton = new rengine::SdlBackend(); \
        return singleton;                                                  \
    }


RENGINE_END_NAMESPACE
