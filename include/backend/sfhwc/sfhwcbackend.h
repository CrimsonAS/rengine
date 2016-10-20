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

#include <sys/time.h>

RENGINE_BEGIN_NAMESPACE

void SfHwcBackend::quit()
{
    logi << __PRETTY_FUNCTION__ << std::endl;
    m_running = false;
}

Surface *SfHwcBackend::createSurface(SurfaceInterface *iface)
{
    // We only allow one surface, the output window..
    assert(!surface);

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
                    surface = new SfHwcSurface(iface, this, size);
                    surface->m_vsyncDelta = values[0] / 1000000.0;
                    surface->m_dpi = dpi;
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

    assert(surface);
    return surface;
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

    pointerState.down = false;
    touchDevice = new SfHwcTouchDevice();
    touchDevice->initialize("/dev/touchscreen");

    char *overridePrediction = std::getenv("RENGINE_TOUCH_PREDICTION");
    if (overridePrediction)
        m_touchPrediction = std::max<float>(0.0f, std::min<float>(500.0f, atof(overridePrediction)));
    logi << "Touch Device: " << std::endl;
    logi << " - max points..........: " << RENGINE_MAX_TOUCH_POINTS << " (compile time)" << std::endl;
    logi << " - # frames predicted .: " << m_touchPrediction << std::endl;
}

inline void SfHwcBackend::processEvents()
{
    timeval halfAFrame;
    halfAFrame.tv_sec = 0;
    halfAFrame.tv_usec = 8 * 1000;

    if (surface && surface->m_iface) {
        surface->m_iface->onRender();

        updateTouch();

        // Upon reaching end of a frame, enter dormancy for a while..
        select(0, 0, 0, 0, &halfAFrame);
    }
}

inline void SfHwcBackend::cb_vsync(int display, int64_t)
{
    // logi << "hit vsync.." << std::endl;
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    m_vsyncMutex.lock();
    m_vsyncTime = t.tv_sec + t.tv_nsec / 1000000000.0;
    m_vsyncMutex.unlock();
}

// inline vec2 sfhwcbackend_predictTouchPoint(const SfHwcTouchDevice::Contact &c, double lastVsync)
// {
//     vec2 curContact(c.x, c.y);
//     vec2 lastContact(c.lx, c.ly);

//     double nowTime = lastVsync + 0.5/60.0;
//     double curTime = c.t.tv_sec + c.t.tv_usec / 1000000.0;
//     double lastTime = c.lt.tv_sec + c.lt.tv_usec / 1000000.0;
//     // double nowTime += 0.01666 - fmod(nowTime, 0.01666);

//     vec2 velocity = (curContact - lastContact) / (curTime - lastTime);

//     vec2 predPoint = velocity * (nowTime - curTime) + curContact;

//     vec2 d = predPoint - curContact;
//     printf("touch times: now=%f, c.t=%f, c.lt=%f, velocity=%f,%f -- dt=%f, delta=(%6.2f,%6.2f)\n",
//            nowTime, curTime, lastTime, velocity.x, velocity.y,
//            nowTime - curTime,
//            d.x, d.y);

//     // printf(" - compensation: %f, dy=%f, dt=%f\n", sqrt(d.x * d.x + d.y * d.y), d.y, nowTime - curTime);

//     return predPoint;
// }

inline vec2 SfHwcBackend::predictPointerState(vec2 pos, PointerState *pointerState)
{
    vec2 last(pointerState->x.value(), pointerState->y.value());
    vec2 current(pointerState->x.update(pos.x), pointerState->y.update(pos.y));

    vec2 velocity(pointerState->vx.value(), pointerState->vy.value());
    // logi << " -- prediction" << velocity * m_touchPrediction / 1000.0f << std::endl;

    return current + velocity * m_touchPrediction / 1000.0f;
}

static vec2 sfhwc_backend_contact_velocity(const SfHwcTouchDevice::Contact &c)
{
    double t = c.t.tv_sec + c.t.tv_usec / 1000000.0;
    double lt = c.lt.tv_sec + c.lt.tv_usec / 1000000.0;
    // printf(" -- update velocity: t=%f->%f, pos=%d,%d, last=%d,%d\n",
    //        lt, t,
    //        c.x, c.y, c.lx, c.ly);
    if (t == lt)
        return vec2(0, 0);
    double dt = t - lt;
    return vec2((c.x - c.lx) / dt, (c.y - c.ly) / dt);
}

inline void SfHwcBackend::updateTouch()
{
    touchDevice->lock();
    Event::Type type = Event::Invalid;
    const SfHwcTouchDevice::State &s = touchDevice->state();
    if (s.count) {
        for (int i=0; i<RENGINE_MAX_TOUCH_POINTS; ++i) {
            if (s.contacts[i].id >= 0) {
                const SfHwcTouchDevice::Contact &c = s.contacts[i];
                double timestamp = c.t.tv_sec + c.t.tv_usec / 1000000.0;
                if (!pointerState.down) {
                    type = Event::PointerDown;
                    pointerState.down = true;
                    pointerState.x.initialize(c.x);
                    pointerState.y.initialize(c.y);
                    pointerState.pos = vec2(c.x, c.y);
                    vec2 v = sfhwc_backend_contact_velocity(c);
                    pointerState.vx.initialize(v.x);
                    pointerState.vy.initialize(v.y);
                }
                else {
                    type = Event::PointerMove;
                    pointerState.pos = predictPointerState(vec2(c.x, c.y), &pointerState);
                    vec2 v = sfhwc_backend_contact_velocity(c);
                    if (pointerState.timestamp == timestamp)
                        v = vec2(0.0f);
                    pointerState.vx.update(v.x);
                    pointerState.vy.update(v.y);
                }
                pointerState.timestamp = timestamp;
                break;
            }
        }
    } else if (pointerState.down) {
        type = Event::PointerUp;
        pointerState.down = false;
    }
    touchDevice->unlock();

    // if (pointerState.down) {
    //     printf("updateTouch: pos=%6.1f,%6.1f; velocity=%6.1f,%6.1f\n",
    //            pointerState.x.value(),
    //            pointerState.y.value(),
    //            pointerState.vx.value(),
    //            pointerState.vy.value()
    //         );
    // }

    if (type != Event::Invalid) {
        PointerEvent pe(type);
        pe.initialize(pointerState.pos);
        surface->m_iface->onEvent(&pe);
    }
}



RENGINE_END_NAMESPACE
