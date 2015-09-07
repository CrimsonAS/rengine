#pragma once

RENGINE_BEGIN_NAMESPACE

SfHwcSurface::SfHwcSurface(SurfaceInterface *iface, SfHwcBackend *backend)
	: m_iface(iface)
	, m_backend(backend)
	, m_vsyncDelta(0)
{
	setSurfaceToInterface(iface);
	initHwc();
}

void SfHwcSurface::hide() { std::cout << __PRETTY_FUNCTION__<< std::endl; }

void SfHwcSurface::show() { std::cout << __PRETTY_FUNCTION__<< std::endl; }

bool SfHwcSurface::makeCurrent() { std::cout << __PRETTY_FUNCTION__<< std::endl; return false; }

bool SfHwcSurface::swapBuffers() { std::cout << __PRETTY_FUNCTION__<< std::endl; return false; }

vec2 SfHwcSurface::size() const
{
	return m_size;
}

void SfHwcSurface::requestRender() { std::cout << __PRETTY_FUNCTION__<< std::endl; }

void SfHwcSurface::initHwc()
{
    const uint32_t DISPLAY_ATTRIBUTES[] = {
        HWC_DISPLAY_VSYNC_PERIOD,
        HWC_DISPLAY_WIDTH,
        HWC_DISPLAY_HEIGHT,
        HWC_DISPLAY_DPI_X,
        HWC_DISPLAY_DPI_Y,
        HWC_DISPLAY_NO_ATTRIBUTE,
    };

	hwc_composer_device_1_t *hwc = m_backend->hwcDevice;

    for (int display=0; display<HWC_NUM_DISPLAY_TYPES; ++display) {
        std::cout << " - display[" << display << "]" << std::endl;
        unsigned int configs[10];
        size_t configCount = 10;
        if (hwc->getDisplayConfigs(hwc, display, configs, &configCount) == 0) {
            for (uint32_t config=0; config<configCount; ++config) {
                int32_t cfg = configs[config];
                std::cout << "   - config: " << cfg << std::endl;
                int32_t values[5];
                hwc->getDisplayAttributes(hwc, display, cfg, DISPLAY_ATTRIBUTES, values);
                m_vsyncDelta = values[0] / 1000000.0;
                m_size = vec2(values[1], values[2]);
                m_dpi = vec2(values[3] / 1000.0f, values[4] / 1000.0f);
                std::cout << "     - VSync .: " << m_vsyncDelta << " ms" << std::endl;
                std::cout << "     - sSize .: " << (int) m_size.x << "x" << (int) m_size.y << std::endl;
                std::cout << "     - dpi ...: " << m_dpi.x << ", " << m_dpi.y << std::endl;
            }
        }
    }
}

RENGINE_END_NAMESPACE