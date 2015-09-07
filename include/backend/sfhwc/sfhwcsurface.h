#pragma once

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

class SfHwcSurface : public Surface
{
public:
	SfHwcSurface(SurfaceInterface *iface);
    void hide() override;
    void show() override;
    bool makeCurrent() override;
    bool swapBuffers() override;
	vec2 size() const override;
    void requestRender() override;

    SurfaceInterface *m_iface;
};

SfHwcSurface::SfHwcSurface(SurfaceInterface *iface)
{
	setSurfaceToInterface(iface);
}

void SfHwcSurface::hide() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
void SfHwcSurface::show() { std::cout << __PRETTY_FUNCTION__<< std::endl; }
bool SfHwcSurface::makeCurrent() { std::cout << __PRETTY_FUNCTION__<< std::endl; return false; }
bool SfHwcSurface::swapBuffers() { std::cout << __PRETTY_FUNCTION__<< std::endl; return false; }
vec2 SfHwcSurface::size() const { return vec2(720, 1280); }
void SfHwcSurface::requestRender() { std::cout << __PRETTY_FUNCTION__<< std::endl; }


