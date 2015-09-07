#pragma once

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

RENGINE_BEGIN_NAMESPACE

class SfHwcSurface;
class SfHwcBackend;

class SfHwcSurface : public Surface
{
public:
	SfHwcSurface(SurfaceInterface *iface, SfHwcBackend *backend);

	void initHwc();

    void hide() override;
    void show() override;
    bool makeCurrent() override;
    bool swapBuffers() override;
	vec2 size() const override;
    void requestRender() override;

    SurfaceInterface *m_iface;
    SfHwcBackend *m_backend;

    double m_vsyncDelta;
    vec2 m_size;
    vec2 m_dpi;
};

class SfHwcBackend : public Backend
{
public:
	SfHwcBackend();

    void quit() override;
    void run() override;

    Surface *createSurface(SurfaceInterface *) override;

    Renderer *createRenderer(Surface *surface) override {
    	OpenGLRenderer *renderer = new OpenGLRenderer();
    	renderer->setTargetSurface(surface);
    	return renderer;
    }

    SfHwcSurface *surface = nullptr;
	hw_module_t *hwcModule = 0;
	hwc_composer_device_1_t *hwcDevice = 0;
};

#define RENGINE_BACKEND_DEFINE                               \
    Backend *Backend::get() {                          		 \
        static SfHwcBackend *singleton = new SfHwcBackend(); \
        return singleton;                              		 \
    }                                                  		 \

RENGINE_END_NAMESPACE

// Implementations, included outside the namespace
#include "sfhwcsurface.h"
#include "sfhwcbackend.h"

