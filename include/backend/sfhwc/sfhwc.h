/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    Copyright (c) 2016, Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
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

#include <thread>
#include <mutex>

struct input_event;
struct mtdev;

RENGINE_BEGIN_NAMESPACE

class SfHwcSurface;
class SfHwcBackend;
class SfHwcBuffer;
class SfHwcTouchDevice;

class SfHwcSurface : public SurfaceBackendImpl, public HWComposerNativeWindow
{
public:
    SfHwcSurface(Surface *iface, SfHwcBackend *backend, vec2 size);

    void initHwc();
    void initEgl();

    void hide() override;
    void show() override;
    bool beginRender() override;
    bool commitRender() override;
    vec2 size() const override;
    void requestSize(vec2) override { logd << "resizing is not supported on this backend" << std::endl; }

    void requestRender() override;

    // ### Dummy values to make to make it compile!!!
    vec2 dpi() const override { return vec2(200, 200); }

    Renderer *createRenderer() override {
        OpenGLRenderer *renderer = new OpenGLRenderer();
        renderer->setTargetSurface(m_surface);
        return renderer;
    }

    void present(HWComposerNativeWindowBuffer *buffer) override;
    void presentAsOverlay(HWComposerNativeWindowBuffer *buffer);
    void presentAsFramebufferTarget(HWComposerNativeWindowBuffer *buffer);

    Surface *m_surface;
    SfHwcBackend *m_backend;

    std::list<SfHwcBuffer *> m_frameBuffers;

    double m_vsyncDelta;
    vec2 m_size;
    vec2 m_dpi;

    bool m_useOverlay;

    hwc_display_contents_1_t *m_hwcList;

    EGLDisplay m_eglDisplay;
    EGLSurface m_eglSurface;
    EGLContext m_eglContext;
};

class SfHwcBackend : public Backend, public hwc_procs_t
{
public:
    SfHwcBackend();

    void processEvents() override;

    void updateTouch();

    SurfaceBackendImpl *createSurface(Surface *surface) override;
    void destroySurface(Surface *surface, SurfaceBackendImpl *impl) override;

    void cb_invalidate() const { logw << std::endl; }
    void cb_vsync(int display, int64_t timestamp);
    void cb_hotplug(int display, int connected) const { logw << "display=" << display << ", connected=" << connected << std::endl; }

    virtual void notifyPendingEvents() { }

    SfHwcSurface *hwcSurface = nullptr;
    gralloc_module_t *grallocModule = 0;
    alloc_device_t *allocDevice = 0;
    hw_module_t *hwcModule = 0;
    hwc_composer_device_1_t *hwcDevice = 0;

    SfHwcTouchDevice *touchDevice;
    struct PointerState {
        KalmanFilter2D x;
        KalmanFilter2D y;
        vec2 pos;
        int id;
    } pointerState;

    float m_touchPrediction = 0.0f;
    float m_touchRate = 0.0f;

    bool m_running = true;

    vec2 predictPointerState(vec2 pos, PointerState *pointerState);
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

    int fence() const { return m_fence; }
    void setFence(int fd) { m_fence = fd; }

private:
    gralloc_module_t *grallocModule() const { return m_backend->grallocModule; }
    alloc_device_t *allocDevice() const { return m_backend->allocDevice; }

    SfHwcBackend *m_backend;
    buffer_handle_t m_handle;
    void *m_bits;
    int m_stride;
    int m_width;
    int m_height;
    int m_fence;
};

#ifndef RENGINE_MAX_TOUCH_POINTS
#define RENGINE_MAX_TOUCH_POINTS 10
#endif

#ifndef RENGINE_MAX_TOUCH_HISTORY
#define RENGINE_MAX_TOUCH_HISTORY 4
#endif

class SfHwcTouchDevice
{
public:
    SfHwcTouchDevice();
    ~SfHwcTouchDevice() { close(); }

    bool initialize(const char *device);
    void close();

    int minX() const { return m_minX; }
    int maxX() const { return m_maxX; }
    int minY() const { return m_minY; }
    int maxY() const { return m_maxY; }
    int minPressure() const { return m_minPressure; }
    int maxPressure() const { return m_maxPressure; }

    const std::string &name() const { return m_name; }

    int fd() const { return m_fd; }

    void run();

    struct Contact
    {
        int id;
        int x;
        int y;
    };

    struct State
    {
        Contact contacts[RENGINE_MAX_TOUCH_POINTS];
        int count;
        timeval time;

        bool isValid() const { return count > 0 && time.tv_sec > 0 && time.tv_usec > 0; }
    };

    void lock();
    void unlock();

    const State &state(int historical = 0) const {
        assert(historical >= 0);
        assert(historical < RENGINE_MAX_TOUCH_HISTORY);
        if (historical != 0) {
            int index = m_stateIndex - historical;
            if (index < 0)
                index += RENGINE_MAX_TOUCH_HISTORY;
            return m_state[index];
        }
        return m_state[m_stateIndex];
    }

private:
    void readEvent(const input_event &e);

    mtdev *m_dev = 0;
    int m_fd = -1;

    int m_minX = -1;
    int m_maxX = -1;
    int m_minY = -1;
    int m_maxY = -1;
    int m_minPressure = -1;
    int m_maxPressure = -1;

    int m_slot = 0;
    State m_pending;
    State m_state[RENGINE_MAX_TOUCH_HISTORY];
    int m_stateIndex = 0;

    std::thread m_thread;
    std::mutex m_mutex;

    std::string m_name;
};


#define RENGINE_BACKEND rengine::SfHwcBackend

RENGINE_END_NAMESPACE

// Implementations, included outside the namespace
#include "sfhwcsurface.h"
#include "sfhwcbackend.h"
#include "sfhwcbuffer.h"
#include "sfhwctouchdevice.h"

