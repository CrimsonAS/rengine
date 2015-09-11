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

SfHwcBackend::SfHwcBackend()
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
}

void SfHwcBackend::run()
{
    logi << __PRETTY_FUNCTION__
         << "; running=" << m_running
         << "; surface=" << surface
         << "; iface=" << (surface ? surface->m_iface : nullptr)
         << std::endl;
    while (m_running && surface && surface->m_iface)
        surface->m_iface->onRender();
}



RENGINE_END_NAMESPACE