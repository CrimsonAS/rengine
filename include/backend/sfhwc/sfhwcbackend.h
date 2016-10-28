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

#include <sys/time.h>

RENGINE_BEGIN_NAMESPACE

SurfaceBackendImpl *SfHwcBackend::createSurface(Surface *surface)
{
    // We only allow one surface, the output window..
    assert(!hwcSurface);

    const uint32_t DISPLAY_ATTRIBUTES[] = {
        HWC_DISPLAY_VSYNC_PERIOD,
        HWC_DISPLAY_WIDTH,
        HWC_DISPLAY_HEIGHT,
        HWC_DISPLAY_DPI_X,
        HWC_DISPLAY_DPI_Y,
        HWC_DISPLAY_NO_ATTRIBUTE,
    };
    for (int display=0; display<HWC_NUM_DISPLAY_TYPES; ++display) {
        logi << " - display[" << display << "]" << std::endl;
        unsigned int configs[10];
        size_t configCount = 10;
        if (hwcDevice->getDisplayConfigs(hwcDevice, display, configs, &configCount) == 0) {
            for (uint32_t config=0; config<configCount; ++config) {
                int32_t cfg = configs[config];
                logi << "   - config: " << cfg << std::endl;
                int32_t values[5];
                hwcDevice->getDisplayAttributes(hwcDevice, display, cfg, DISPLAY_ATTRIBUTES, values);
                double vsyncDelta = values[0] / 1000000.0;
                vec2 size = vec2(values[1], values[2]);
                vec2 dpi = vec2(values[3] / 1000.0f, values[4] / 1000.0f);

                logi << "     - VSync .: " << vsyncDelta << " ms" << std::endl;
                logi << "     - sSize .: " << (int) size.x << "x" << (int) size.y << std::endl;
                logi << "     - dpi ...: " << dpi.x << ", " << dpi.y << std::endl;

                if (config == 0) {
                    hwcSurface = new SfHwcSurface(surface, this, size);
                    hwcSurface->m_vsyncDelta = values[0] / 1000000.0;
                    hwcSurface->m_dpi = dpi;
                }
            }
        }
    }

    int value;
    hwcDevice->query(hwcDevice, HWC_BACKGROUND_LAYER_SUPPORTED, &value);
    logi << " - Background Layer Support: " << (value != 0 ? "yes" : "no") << std::endl;
    hwcDevice->query(hwcDevice, HWC_DISPLAY_TYPES_SUPPORTED, &value);
    logi << " - Supported Display Types:"
         << ((value & HWC_DISPLAY_PRIMARY_BIT) ? " primary" : "")
         << ((value & HWC_DISPLAY_EXTERNAL_BIT) ? " external" : "")
         << ((value & HWC_DISPLAY_VIRTUAL_BIT) ? " virtual" : "")
         << std::endl;

    assert(hwcSurface);
    return hwcSurface;
}

inline void SfHwcBackend::destroySurface(Surface *surface, SurfaceBackendImpl *impl)
{
    delete impl;
}

inline void sfhwc_hooks_invalidate(const hwc_procs_t *procs) {
    static_cast<const SfHwcBackend *>(procs)->cb_invalidate();
}

inline void sfhwc_hooks_vsync(const hwc_procs_t *procs, int display, int64_t timestamp) {
    const_cast<SfHwcBackend *>(static_cast<const SfHwcBackend *>(procs))->cb_vsync(display, timestamp);
}

inline void sfhwc_hooks_hotplug(const hwc_procs_t *procs, int display, int connected) {
    static_cast<const SfHwcBackend *>(procs)->cb_hotplug(display, connected);
}

inline SfHwcBackend::SfHwcBackend()
{
    if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t **) &grallocModule) != 0 || !grallocModule) {
        loge << "failed to open gralloc module" << std::endl;
        return;
    }
    if (gralloc_open((const hw_module_t *) grallocModule, &allocDevice) != 0 || !allocDevice) {
        loge << "failed to open alloc device" << std::endl;
        return;
    }
    logi << "Gralloc:" << std::endl;
    logi << " - gralloc module ......: " << grallocModule << std::endl;
    logi << " - alloc device ........: " << allocDevice << std::endl;

    if (hw_get_module(HWC_HARDWARE_MODULE_ID, (const hw_module_t **)(&hwcModule)) != 0 || !hwcModule) {
        loge << ": failed to open module" << std::endl;
        return;
    }
    logi << "Hardware Composer Module:" << std::endl;
    logi << " - Address .............: " << hwcModule << std::endl;
    logi << " - Module API Version ..: " << std::hex << hwcModule->module_api_version << std::endl;
    logi << " - HAL API Version .....: " << std::hex << hwcModule->hal_api_version << std::endl;
    logi << " - Identifier ..........: " << hwcModule->id << std::endl;
    logi << " - Name ................: " << hwcModule->name << std::endl;
    logi << " - Author ..............: " << hwcModule->author << std::endl;

    hw_device_t *device = 0;
    if (hwcModule->methods->open(hwcModule, HWC_HARDWARE_COMPOSER, (hw_device_t **) &device) != 0 || !device) {
        std::cerr << "error: " << __PRETTY_FUNCTION__ << ": failed to open device" << std::endl;
        return;
    }
    hwcDevice = (hwc_composer_device_1_t *) device;
    logi << "Hardware Composer Device" << std::endl;
    logi << " - version ............: " << std::hex << hwcDevice->common.version << std::endl;
    logi << " - module .............: " << hwcDevice->common.module << std::endl;
    logi << " - tag ................: " << std::hex << hwcDevice->common.tag << std::dec << std::endl;
    logi << " - composer/device ....: " << hwcDevice << std::endl;

#ifndef NDEBUG
    if (hwcDevice->dump) {
        int maxLength = 64 * 1024;
        char *bytes = (char *) malloc(maxLength);
        hwcDevice->dump(hwcDevice, bytes, maxLength);
        logi << "Hwc Device Dump ...: " << bytes << std::endl;
        free(bytes);
    } else {
        logi << "Hwc Device Dump ...: n/a" << std::endl;
    }
#endif

    // Register callbacks
    invalidate = sfhwc_hooks_invalidate;
    vsync = sfhwc_hooks_vsync;
    hotplug = sfhwc_hooks_hotplug;
    hwcDevice->registerProcs(hwcDevice, this);

    pointerState.id = -1;
    touchDevice = new SfHwcTouchDevice();
    touchDevice->initialize("/dev/touchscreen");

    char *overridePrediction = std::getenv("RENGINE_TOUCH_PREDICTION");
    if (overridePrediction)
        m_touchPrediction = std::max<float>(0.0f, std::min<float>(500.0f, atof(overridePrediction))) / 1000.0f;
    logi << "Touch Device: " << std::endl;
    logi << " - max points.............: " << RENGINE_MAX_TOUCH_POINTS << " (compile time)" << std::endl;
    logi << " - prediction in seconds .: " << m_touchPrediction << std::endl;
}

inline void SfHwcBackend::processEvents()
{
    timeval halfAFrame;
    halfAFrame.tv_sec = 0;
    halfAFrame.tv_usec = 8 * 1000;

    if (hwcSurface && hwcSurface->m_surface) {
        hwcSurface->m_surface->onRender();

        updateTouch();

        // Upon reaching end of a frame, enter dormancy for a while..
        select(0, 0, 0, 0, &halfAFrame);
    }
}

inline void SfHwcBackend::cb_vsync(int display, int64_t)
{
    // logi << "vsync.." << std::endl;
}

inline double sfhwc_timeval_to_seconds(timeval t) {
    return t.tv_sec + t.tv_usec / 1000000.0;
}

inline void SfHwcBackend::updateTouch()
{
    // m_vsyncMutex.lock();
    // double vsyncTime = m_vsyncTime;
    // m_vsyncMutex.unlock();
    double vsyncDelta = hwcSurface->m_vsyncDelta / 1000.0;

    touchDevice->lock();
    SfHwcTouchDevice::State state = touchDevice->state();
    SfHwcTouchDevice::State lastState = touchDevice->state(1);
    touchDevice->unlock();

    if (state.count) {

        // Calculate an estimate of the time between touch samples. We use the
        // current and last touch sample set to figure this out and use the
        // value to calibrate our estimate over time. We do this rather than
        // relying directly on the delta between current and last as there
        // is quite a large margin of error when a system is under load.
        double touchTime = sfhwc_timeval_to_seconds(state.time);
        double lastTouchTime = sfhwc_timeval_to_seconds(lastState.time);
        double touchTimeDelta = touchTime - lastTouchTime;
        if (m_touchRate == 0) {
            // Set initial value..
            if (touchTimeDelta < vsyncDelta)
                m_touchRate = touchTimeDelta;
            else
                m_touchRate = vsyncDelta / 2.0;
        } else {
            // update our estimate. We ignore the delta if it is larger than a
            // vsync cycle which we interpret as either a lack of events for some
            // period of time (aka, no movement), or that there was some severe
            // fluctuation which should also be ignored..
            if (touchTimeDelta < vsyncDelta) {
                // The estimate update formula is meant to give weight to the
                // current value (which is thus assumed to tangent the optimal
                // estimate) while providing updates over time and giving only a
                // small correction effect from errors which stray from the
                // optimum.
                m_touchRate = sqrt(m_touchRate * m_touchRate * 0.9 + touchTimeDelta * touchTimeDelta * 0.1);
            }
        }
        // std::cout << "touch rate estimate is: " << m_touchRate << std::endl;

        Event::Type type = Event::Invalid;
        for (int i=0; i<RENGINE_MAX_TOUCH_POINTS; ++i) {
            SfHwcTouchDevice::Contact c = state.contacts[i];
            if (c.id > 0) {
                vec2 velocity;
                if (lastState.isValid()) {
                    for (int j=0; j<RENGINE_MAX_TOUCH_POINTS; ++j) {
                        SfHwcTouchDevice::Contact lc = lastState.contacts[i];
                        if (lc.id == c.id) {
                            velocity = vec2(c.x - lc.x, c.y - lc.y) / m_touchRate;
                            break;
                        }
                    }
                }

                if (pointerState.id != c.id) {
                    type = Event::PointerDown;
                    pointerState.id = c.id;
                    pointerState.x.initialize(c.x, velocity.x);
                    pointerState.y.initialize(c.y, velocity.y);
                } else {
                    type = Event::PointerMove;
                    pointerState.x.update(c.x, velocity.x, vsyncDelta);
                    pointerState.y.update(c.y, velocity.y, vsyncDelta);
                }
            }
        }

        // if (pointerState.id > 0) {
        //     printf("updateTouch: pos=%6.1f,%6.1f; velocity=%6.1f,%6.1f\n",
        //            pointerState.x.position(),
        //            pointerState.y.position(),
        //            pointerState.x.velocity(),
        //            pointerState.y.velocity());
        // }

        vec2 pos(pointerState.x.position(), pointerState.y.position());
        vec2 vel(pointerState.x.velocity(), pointerState.y.velocity());
        pointerState.pos = pos + vel * m_touchPrediction;

        assert(type != Event::Invalid);
        PointerEvent pe(type);
        pe.initialize(pointerState.pos);
        hwcSurface->m_surface->onEvent(&pe);

    } else if (pointerState.id > 0) {
        PointerEvent pe(Event::PointerUp);
        pe.initialize(pointerState.pos);
        hwcSurface->m_surface->onEvent(&pe);
        pointerState.id = -1;
    }
}



RENGINE_END_NAMESPACE
