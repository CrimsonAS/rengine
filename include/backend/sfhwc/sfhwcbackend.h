#pragma once

RENGINE_BEGIN_NAMESPACE

class SfHwcSurface;

class SfHwcBackend : public Backend
{
public:
    void quit() override;
    void run() override;

    Surface *createSurface(SurfaceInterface *) override;

    Renderer *createRenderer(Surface *surface) override {
    	OpenGLRenderer *renderer = new OpenGLRenderer();
    	renderer->setTargetSurface(surface);
    	return renderer;
    }

    SfHwcSurface *m_surface = nullptr;
};

class SfHwcSurface : public Surface
{
public:
	SfHwcSurface(SurfaceInterface *iface);
    void hide() override;
    void show() override;
    bool makeCurrent() override;
    bool swapBuffers() override;
	vec2 size() const override;
    void requestRender() override;

    SurfaceInterface *m_iface;
};

void SfHwcBackend::quit() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
void SfHwcBackend::run() { std::cout << __PRETTY_FUNCTION__<< std::endl; }

Surface *SfHwcBackend::createSurface(SurfaceInterface *iface)
{
	// We only allow one surface, the output window..
	assert(!m_surface);
	m_surface = new SfHwcSurface(iface);
	return m_surface;
}

SfHwcSurface::SfHwcSurface(SurfaceInterface *iface)
{
	setSurfaceToInterface(iface);
}

void SfHwcSurface::hide() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
void SfHwcSurface::show() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
bool SfHwcSurface::makeCurrent() { std::cout << __PRETTY_FUNCTION__<< std::endl; return false; }
bool SfHwcSurface::swapBuffers() { std::cout << __PRETTY_FUNCTION__<< std::endl; return false; }
vec2 SfHwcSurface::size() const { return vec2(720, 1280); }
void SfHwcSurface::requestRender() { std::cout << __PRETTY_FUNCTION__<< std::endl; }



#define RENGINE_BACKEND_DEFINE                               \
    Backend *Backend::get() {                          		 \
        static SfHwcBackend *singleton = new SfHwcBackend(); \
        return singleton;                              		 \
    }                                                  		 \

RENGINE_END_NAMESPACE