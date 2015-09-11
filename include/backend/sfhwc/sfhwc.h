/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    Copyright (c) 2015, Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
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

#pragma once

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include "hwcomposer_window.h"

RENGINE_BEGIN_NAMESPACE

class SfHwcSurface;
class SfHwcBackend;
class SfHwcBuffer;

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
    gralloc_module_t *grallocModule = 0;
    alloc_device_t *allocDevice = 0;
	hw_module_t *hwcModule = 0;
	hwc_composer_device_1_t *hwcDevice = 0;

    bool m_running = true;
};

class SfHwcBuffer
{
public:
    SfHwcBuffer(SfHwcBackend *backend,
                int w = 0,
                int h = 0,
                int format = HAL_PIXEL_FORMAT_RGBA_8888,
                int usage = GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_SW_WRITE_RARELY | GRALLOC_USAGE_HW_FB);
    ~SfHwcBuffer();

    bool isCreated() const { return m_handle != 0; }
    bool isLocked() const { return m_bits != 0; }

    void release();

    void lock();
    void unlock();
    void *bits() { return m_bits; }

    int width() const { return m_width; }
    int height() const { return m_height; }
    buffer_handle_t handle() { return m_handle; }

    void fillWithCrap();

private:
    gralloc_module_t *grallocModule() const { return m_backend->grallocModule; }
    alloc_device_t *allocDevice() const { return m_backend->allocDevice; }

    SfHwcBackend *m_backend;
    buffer_handle_t m_handle;
    void *m_bits;
    int m_stride;
    int m_width;
    int m_height;
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
#include "sfhwcbuffer.h"

