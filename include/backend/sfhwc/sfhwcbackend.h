#pragma once

RENGINE_BEGIN_NAMESPACE

#include "sfhwcsurface.h"

class SfHwcSurface;
class SfHwcBackend;

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

void SfHwcBackend::quit() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
void SfHwcBackend::run() { std::cout << __PRETTY_FUNCTION__<< std::endl; }

Surface *SfHwcBackend::createSurface(SurfaceInterface *iface)
{
	// We only allow one surface, the output window..
	assert(!m_surface);
	m_surface = new SfHwcSurface(iface);
	return m_surface;
}


#define RENGINE_BACKEND_DEFINE                               \
    Backend *Backend::get() {                          		 \
        static SfHwcBackend *singleton = new SfHwcBackend(); \
        return singleton;                              		 \
    }                                                  		 \

RENGINE_END_NAMESPACE