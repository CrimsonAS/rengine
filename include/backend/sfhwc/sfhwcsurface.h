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

#include <cstring>

RENGINE_BEGIN_NAMESPACE

inline const char *sfhwc_decode_egl_error(EGLint error)
{
	switch (error) {
	case EGL_SUCCESS: return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
	case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
	default:
		break;
	}
	return "unknown error";
}

inline void sfhwc_dump_display_contents(hwc_display_contents_1_t *dc)
{
    printf(" - displayContents, retireFence=%d, outbuf=%p, outAcqFence=%d, flags=%x, numLayers=%d\n",
            dc->retireFenceFd,
            dc->outbuf,
            dc->outbufAcquireFenceFd,
            (int) dc->flags,
            (int) dc->numHwLayers);
    for (unsigned int i=0; i<dc->numHwLayers; ++i) {
        const hwc_layer_1_t &l = dc->hwLayers[i];
        printf("    - layer comp=%x, hints=%x, flags=%x, handle=%p, transform=%x, blending=%x, "
                "src=(%d %d - %dx%d), dst=(%d %d - %dx%d), afd=%d, rfd=%d, a=%d, "
                "region=(%d %d - %dx%d)\n",
                l.compositionType, l.hints, l.flags, l.handle, l.transform, l.blending,
                (int) l.sourceCropf.left, (int) l.sourceCropf.top, (int) l.sourceCropf.right, (int) l.sourceCropf.bottom,
                l.displayFrame.left, l.displayFrame.top, l.displayFrame.right, l.displayFrame.bottom,
                l.acquireFenceFd, l.releaseFenceFd, l.planeAlpha,
                l.visibleRegionScreen.rects[0].left,
                l.visibleRegionScreen.rects[0].top,
                l.visibleRegionScreen.rects[0].right,
                l.visibleRegionScreen.rects[0].bottom);
    }
}

SfHwcSurface::SfHwcSurface(SurfaceInterface *iface, SfHwcBackend *backend, const vec2 &size)
	: HWComposerNativeWindow(size.x, size.y, HAL_PIXEL_FORMAT_RGBA_8888)
	, m_iface(iface)
	, m_backend(backend)
	, m_vsyncDelta(0)
	, m_size(size)
{
	setSurfaceToInterface(iface);
	initHwc();
	initEgl();
}

void SfHwcSurface::hide()
{
	std::cout << __PRETTY_FUNCTION__<< std::endl;
	hwc_composer_device_1_t *hwc = m_backend->hwcDevice;
	hwc->setPowerMode(hwc, 0, HWC_POWER_MODE_OFF);
	// hwc->blank(hwc, 0, 0);
}

void SfHwcSurface::show()
{
	std::cout << __PRETTY_FUNCTION__<< std::endl;
	hwc_composer_device_1_t *hwc = m_backend->hwcDevice;
	hwc->setPowerMode(hwc, 0, HWC_POWER_MODE_NORMAL);
	// hwc->blank(hwc, 0, 1);
}

bool SfHwcSurface::makeCurrent()
{
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	EGLBoolean ok = eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	if (!ok) {
		std::cerr << "error: " << __PRETTY_FUNCTION__ << ": " << sfhwc_decode_egl_error(eglGetError());
		return false;
	}
	return true;
}

bool SfHwcSurface::swapBuffers()
{
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
    EGLBoolean ok = eglSwapBuffers(m_eglDisplay, m_eglSurface);
    if (!ok) {
		std::cerr << "error: " << __PRETTY_FUNCTION__ << ": " << sfhwc_decode_egl_error(eglGetError());
		return false;
    }
    return true;
}

vec2 SfHwcSurface::size() const
{
	return m_size;
}

void SfHwcSurface::requestRender()
{
	// std::cout << __PRETTY_FUNCTION__<< std::endl;
}

void SfHwcSurface::initHwc()
{
    size_t size = sizeof(hwc_display_contents_1_t) + 2 * sizeof(hwc_layer_1_t);
    m_hwcList = (hwc_display_contents_1_t *) malloc(size);
    memset(m_hwcList, 0, size);

    printf("size: %d, dc=%d, layer=%d\n", size,
    	sizeof(hwc_display_contents_1_t), sizeof(hwc_layer_1_t));

    m_hwcList->retireFenceFd = -1;
    m_hwcList->flags = HWC_GEOMETRY_CHANGED;
    m_hwcList->numHwLayers = 2;

    hwc_layer_1_t *l = &m_hwcList->hwLayers[0];
    l->compositionType = HWC_FRAMEBUFFER;
    l->hints = 0;
    l->flags = 0;
    l->handle = 0;
    l->transform = 0;
    l->blending = HWC_BLENDING_NONE;
    l->sourceCropf.left = 0.0f;
    l->sourceCropf.top = 0.0f;
    l->sourceCropf.right = m_size.x;
    l->sourceCropf.bottom = m_size.y;
    l->displayFrame.left = 0;
    l->displayFrame.top = 0;
    l->displayFrame.right = (int) m_size.x;
    l->displayFrame.bottom = (int) m_size.y;
    l->visibleRegionScreen.numRects = 1;
    l->visibleRegionScreen.rects = &l->displayFrame;
    l->acquireFenceFd = -1;
    l->releaseFenceFd = -1;
    l->planeAlpha = 0xff;

    l = &m_hwcList->hwLayers[1];
    l->compositionType = HWC_FRAMEBUFFER_TARGET;
    l->hints = 0;
    l->flags = 0;
    l->handle = 0;
    l->transform = 0;
    l->blending = HWC_BLENDING_NONE;
    l->sourceCropf.left = 0.0f;
    l->sourceCropf.top = 0.0f;
    l->sourceCropf.right = m_size.x;
    l->sourceCropf.bottom = m_size.y;
    l->displayFrame.left = 0;
    l->displayFrame.top = 0;
    l->displayFrame.right = (int) m_size.x;
    l->displayFrame.bottom = (int) m_size.y;
    l->visibleRegionScreen.numRects = 1;
    l->visibleRegionScreen.rects = &l->displayFrame;
    l->acquireFenceFd = -1;
    l->releaseFenceFd = -1;
    l->planeAlpha = 0xff;

    sfhwc_dump_display_contents(m_hwcList);
}

void SfHwcSurface::initEgl()
{
	EGLint const eglConfigAttributes[] = {
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
	    EGL_ALPHA_SIZE, 8,
	    EGL_NONE
	};

	EGLint const eglContextAttributes[] = {
	    EGL_CONTEXT_CLIENT_VERSION, 2,
	    EGL_NONE
	};

	EGLBoolean result;
	m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	result = eglInitialize(m_eglDisplay, NULL, NULL);
	assert(result);

	int num_config;
    EGLConfig eglConfig;
    result = eglChooseConfig(m_eglDisplay, eglConfigAttributes, &eglConfig, 1, &num_config);
    assert(result);

    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, eglConfig, (EGLNativeWindowType) static_cast<ANativeWindow *>(this), NULL);
    assert(m_eglSurface);

    m_eglContext = eglCreateContext(m_eglDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttributes);
    assert(m_eglContext);

	result = eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	assert(result);

	std::cout << "EGL Configuration:" << std::endl;
	std::cout << " - Display .........: " << m_eglDisplay << std::endl;
	std::cout << " - Config ..........: " << eglConfig << std::endl;
	std::cout << " - Surface .........: " << m_eglSurface << std::endl;
	std::cout << " - Context .........: " << m_eglContext << std::endl;
	std::cout << " - EGL_VENDOR ......: " << eglQueryString(m_eglDisplay, EGL_VENDOR) << std::endl;
	std::cout << " - EGL_VERSION .....: " << eglQueryString(m_eglDisplay, EGL_VERSION) << std::endl;
	std::cout << " - EGL_CLIENT_APIS .: " << eglQueryString(m_eglDisplay, EGL_CLIENT_APIS) << std::endl;
	std::cout << " - EGL_EXTENSIONS ..: " << eglQueryString(m_eglDisplay, EGL_EXTENSIONS) << std::endl;
	std::cout << " - GL_VERSION ......: " << glGetString(GL_VERSION) << std::endl;
	std::cout << " - GL_VENDOR .......: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << " - GL_RENDERER .....: " << glGetString(GL_RENDERER) << std::endl;

	int r, g, b, a, d, s;
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_ALPHA_SIZE, &a);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_RED_SIZE, &r);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_GREEN_SIZE, &g);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_BLUE_SIZE, &b);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_DEPTH_SIZE, &d);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_STENCIL_SIZE, &s);
	std::cout << " - RGBA Buffers ....: " << r << " " << g << " " << b << " " << a << std::endl;
	std::cout << " - Depth / Stencil .: " << d << " " << s << std::endl;

	eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


void SfHwcSurface::present(HWComposerNativeWindowBuffer *buffer)
{
	// std::cout << __PRETTY_FUNCTION__ << ": " << buffer << std::endl;

	int status;
	hwc_composer_device_1_t *hwc = m_backend->hwcDevice;

	hwc_layer_1_t &l = m_hwcList->hwLayers[1];
	l.handle = buffer->handle;
	l.acquireFenceFd = getFenceBufferFd(buffer);
	l.releaseFenceFd = -1;

	status = hwc->prepare(hwc, 1, &m_hwcList);
	assert(status == 0);
    // sfhwc_dump_display_contents(m_hwcList);

	status = hwc->set(hwc, 1, &m_hwcList);
	assert(status == 0);

	setFenceBufferFd(buffer, l.releaseFenceFd);

	if (m_hwcList->retireFenceFd != -1) {
		close(m_hwcList->retireFenceFd);
		m_hwcList->retireFenceFd = -1;
	}
}

RENGINE_END_NAMESPACE