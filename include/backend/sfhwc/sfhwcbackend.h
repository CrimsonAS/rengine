#pragma once

RENGINE_BEGIN_NAMESPACE

void SfHwcBackend::quit() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
void SfHwcBackend::run() { std::cout << __PRETTY_FUNCTION__<< std::endl; }

Surface *SfHwcBackend::createSurface(SurfaceInterface *iface)
{
    // We only allow one surface, the output window..
    assert(!surface);
    surface = new SfHwcSurface(iface, this);
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

RENGINE_END_NAMESPACE