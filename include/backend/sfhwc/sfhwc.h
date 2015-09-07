#pragma once

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include "hwcomposer_window.h"

RENGINE_BEGIN_NAMESPACE

class SfHwcSurface;
class SfHwcBackend;

class SfHwcSurface : public Surface, public HWComposerNativeWindow
{
public:
	SfHwcSurface(SurfaceInterface *iface, SfHwcBackend *backend, const vec2 &size);

    void initHwc();
	void initEgl();

    void hide() override;
    void show() override;
    bool makeCurrent() override;
    bool swapBuffers() override;
	vec2 size() const override;
    void requestRender() override;

   	void present(HWComposerNativeWindowBuffer *buffer) override;

    SurfaceInterface *m_iface;
    SfHwcBackend *m_backend;

    double m_vsyncDelta;
    vec2 m_size;
    vec2 m_dpi;

    hwc_display_contents_1_t *m_hwcList;

    EGLDisplay m_eglDisplay;
    EGLSurface m_eglSurface;
    EGLContext m_eglContext;
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

    bool m_running = true;
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

