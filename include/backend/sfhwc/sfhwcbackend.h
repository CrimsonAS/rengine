#pragma once

RENGINE_BEGIN_NAMESPACE

void SfHwcBackend::quit()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
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
        std::cout << " - display[" << display << "]" << std::endl;
        unsigned int configs[10];
        size_t configCount = 10;
        if (hwcDevice->getDisplayConfigs(hwcDevice, display, configs, &configCount) == 0) {
            for (uint32_t config=0; config<configCount; ++config) {
                int32_t cfg = configs[config];
                std::cout << "   - config: " << cfg << std::endl;
                int32_t values[5];
                hwcDevice->getDisplayAttributes(hwcDevice, display, cfg, DISPLAY_ATTRIBUTES, values);
                double vsyncDelta = values[0] / 1000000.0;
                vec2 size = vec2(values[1], values[2]);
                vec2 dpi = vec2(values[3] / 1000.0f, values[4] / 1000.0f);

                std::cout << "     - VSync .: " << vsyncDelta << " ms" << std::endl;
                std::cout << "     - sSize .: " << (int) size.x << "x" << (int) size.y << std::endl;
                std::cout << "     - dpi ...: " << dpi.x << ", " << dpi.y << std::endl;

                if (config == 0) {
                    surface = new SfHwcSurface(iface, this, size);
                    surface->m_vsyncDelta = values[0] / 1000000.0;
                    surface->m_dpi = dpi;
                }
            }
        }
    }

    assert(surface);
    return surface;
}

SfHwcBackend::SfHwcBackend()
{
    if (hw_get_module(HWC_HARDWARE_MODULE_ID, (const hw_module_t **)(&hwcModule)) != 0 || !hwcModule) {
        std::cerr << "error: " << __PRETTY_FUNCTION__ << ": failed to open module" << std::endl;
        return;
    }
    std::cout << "Hardware Composer Module:" << std::endl;
    std::cout << " - Address .............: " << hwcModule << std::endl;
    std::cout << " - Module API Version ..: " << std::hex << hwcModule->module_api_version << std::endl;
    std::cout << " - HAL API Version .....: " << std::hex << hwcModule->hal_api_version << std::endl;
    std::cout << " - Identifier ..........: " << hwcModule->id << std::endl;
    std::cout << " - Name ................: " << hwcModule->name << std::endl;
    std::cout << " - Author ..............: " << hwcModule->author << std::endl;

    hw_device_t *device = 0;
    if (hwcModule->methods->open(hwcModule, HWC_HARDWARE_COMPOSER, (hw_device_t **) &device) != 0 || !device) {
        std::cerr << "error: " << __PRETTY_FUNCTION__ << ": failed to open device" << std::endl;
        return;
    }
    hwcDevice = (hwc_composer_device_1_t *) device;
    std::cout << "Hardware Composer Device" << std::endl;
    std::cout << " - version ............: " << std::hex << hwcDevice->common.version << std::endl;
    std::cout << " - module .............: " << hwcDevice->common.module << std::endl;
    std::cout << " - tag ................: " << std::hex << hwcDevice->common.tag << std::dec << std::endl;
    std::cout << " - composer/device ....: " << hwcDevice << std::endl;
}

void SfHwcBackend::run()
{
    std::cout << __PRETTY_FUNCTION__
              << "; running=" << m_running
              << "; surface=" << surface
              << "; iface=" << (surface ? surface->m_iface : nullptr)
              << std::endl;
    while (m_running && surface && surface->m_iface)
        surface->m_iface->onRender();
}



RENGINE_END_NAMESPACE