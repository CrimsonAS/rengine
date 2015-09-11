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

inline void sfhwc_dump_hwc_device(hwc_composer_device_1_t *hwc)
{
    int maxLength = 64 * 1024;
    char *bytes = (char *) malloc(maxLength);
    hwc->dump(hwc, bytes, maxLength);
    std::cout << "Hwc Device Dump ...: " << bytes << std::endl;
    free(bytes);
}

inline void sfhwc_dump_display_contents(hwc_display_contents_1_t *dc)
{
    printf(" - displayContents, retireFence=%d, outbuf=%p, outAcqFence=%d, flags=%x, numLayers=%d, dc=%p\n",
            dc->retireFenceFd,
            dc->outbuf,
            dc->outbufAcquireFenceFd,
            (int) dc->flags,
            (int) dc->numHwLayers,
            dc);
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

inline void sfhwc_initialize_layer(hwc_layer_1_t *l, unsigned compositionType, int x, int y, int w, int h)
{
    l->compositionType = compositionType;
    l->hints = 0;
    l->flags = 0;
    l->handle = 0;
    l->transform = 0;
    l->blending = HWC_BLENDING_NONE;
    l->sourceCropf.left = x;
    l->sourceCropf.top = y;
    l->sourceCropf.right = w;
    l->sourceCropf.bottom = h;
    l->displayFrame.left = x;
    l->displayFrame.top = y;
    l->displayFrame.right = w;
    l->displayFrame.bottom = h;
    l->visibleRegionScreen.numRects = 1;
    l->visibleRegionScreen.rects = &l->displayFrame;
    l->acquireFenceFd = -1;
    l->releaseFenceFd = -1;
    l->planeAlpha = 0xff;
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
	logd << std::endl;
	hwc_composer_device_1_t *hwc = m_backend->hwcDevice;
	hwc->setPowerMode(hwc, 0, HWC_POWER_MODE_OFF);
	// hwc->blank(hwc, 0, 0);
}

void SfHwcSurface::show()
{
	logd << std::endl;
	hwc_composer_device_1_t *hwc = m_backend->hwcDevice;
	hwc->setPowerMode(hwc, 0, HWC_POWER_MODE_NORMAL);
	// hwc->blank(hwc, 0, 0);
}

bool SfHwcSurface::makeCurrent()
{
	logd << std::endl;
	EGLBoolean ok = eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	if (!ok) {
		logw << sfhwc_decode_egl_error(eglGetError()) << std::endl;
		return false;
	}
	return true;
}

bool SfHwcSurface::swapBuffers()
{
	logd << std::endl;
    EGLBoolean ok = eglSwapBuffers(m_eglDisplay, m_eglSurface);
    if (!ok) {
		logw << sfhwc_decode_egl_error(eglGetError()) << std::endl;
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
	logd << std::endl;
}

void SfHwcSurface::initHwc()
{
    size_t size = sizeof(hwc_display_contents_1_t) + 2 * sizeof(hwc_layer_1_t);
    m_hwcList = (hwc_display_contents_1_t *) malloc(size);
    memset(m_hwcList, 0, size);

    m_hwcList->retireFenceFd = -1;
    m_hwcList->flags = HWC_GEOMETRY_CHANGED;
    m_hwcList->numHwLayers = 2;

    sfhwc_initialize_layer(&m_hwcList->hwLayers[0], HWC_FRAMEBUFFER, 0, 0, m_size.x, m_size.y);
    sfhwc_initialize_layer(&m_hwcList->hwLayers[1], HWC_FRAMEBUFFER_TARGET, 0, 0, m_size.x, m_size.y);
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

	EGLBoolean result; (void) result;
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

	logi << "EGL Configuration:" << std::endl;
	logi << " - Display .........: " << m_eglDisplay << std::endl;
	logi << " - Config ..........: " << eglConfig << std::endl;
	logi << " - Surface .........: " << m_eglSurface << std::endl;
	logi << " - Context .........: " << m_eglContext << std::endl;
	logi << " - EGL_VENDOR ......: " << eglQueryString(m_eglDisplay, EGL_VENDOR) << std::endl;
	logi << " - EGL_VERSION .....: " << eglQueryString(m_eglDisplay, EGL_VERSION) << std::endl;
	logi << " - EGL_CLIENT_APIS .: " << eglQueryString(m_eglDisplay, EGL_CLIENT_APIS) << std::endl;
	logi << " - EGL_EXTENSIONS ..: " << eglQueryString(m_eglDisplay, EGL_EXTENSIONS) << std::endl;
	int r, g, b, a, d, s;
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_ALPHA_SIZE, &a);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_RED_SIZE, &r);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_GREEN_SIZE, &g);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_BLUE_SIZE, &b);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_DEPTH_SIZE, &d);
	eglGetConfigAttrib(m_eglDisplay, eglConfig, EGL_STENCIL_SIZE, &s);
	logi << " - RGBA Buffers ....: " << r << " " << g << " " << b << " " << a << std::endl;
	logi << " - Depth / Stencil .: " << d << " " << s << std::endl;
}

void SfHwcSurface::present(HWComposerNativeWindowBuffer *buffer)
{
	logd << "buffer=" << (void *) buffer << std::endl;

	std::cout << "present is here: " << std::endl;

	static SfHwcBuffer *staticBuffer = 0;
	if (!staticBuffer) {
		staticBuffer = new SfHwcBuffer(m_backend, 720, 1280);
		staticBuffer->lock();
		staticBuffer->fillWithCrap();
		staticBuffer->unlock();
	}

	static int counter = 10;
	if (counter > 0) {
		// Question: How come this buffer is only visible in the beginning (and then not the fb target).
		// Is it related to that we touch it on the CPU?
		staticBuffer->lock();
		staticBuffer->fillWithCrap();
		staticBuffer->unlock();
		--counter;
	}

	// int fd = getFenceBufferFd(buffer);
	// if (fd != -1) {
	// 	sync_wait(fd, -1);
	// 	close(fd);
	// }

// */	staticBuffer->lock();
	// unsigned char *bits = (unsigned char *) staticBuffer->bits();
	// m_backend->grallocModule->lock(m_backend->grallocModule,
	// 							   staticBuffer->handle(),
	// 							   GRALLOC_USAGE_SW_WRITE_RARELY,
	// 							   0, 0, 720, 1280,
	// 							   (void **) &bits);
	// assert(bits);
	// staticBuffer->fillWithCrap();
	// // m_backend->grallocModule->unlock(m_backend->grallocModule, staticBuffer->handle());
	// staticBuffer->unlock();

	int status; (void) status;
    hwc_composer_device_1_t *hwc = m_backend->hwcDevice;

    int dcSize = sizeof(hwc_display_contents_1_t) + 2 * sizeof(hwc_layer_1_t);
	hwc_display_contents_1_t *dc = (hwc_display_contents_1_t *) malloc(dcSize);
	memset(dc, 0, sizeof(dcSize));
	dc->numHwLayers = 2;
	dc->flags = HWC_GEOMETRY_CHANGED;
	dc->retireFenceFd = -1;
	dc->outbufAcquireFenceFd = -1;

    sfhwc_initialize_layer(&dc->hwLayers[0], HWC_FRAMEBUFFER, 0, 0, m_size.x, m_size.y);
    sfhwc_initialize_layer(&dc->hwLayers[1], HWC_FRAMEBUFFER_TARGET, 0, 0, m_size.x, m_size.y);

    hwc_layer_1_t *fb = &dc->hwLayers[0];
    fb->handle = staticBuffer->handle();
    fb->blending = HWC_BLENDING_PREMULT;
    fb->acquireFenceFd = -1;
    fb->releaseFenceFd = -1;

    // Question: How come that despite us setting PREMULT (COVERAGE acts the same)
    // on both of our buffers, we have no blending in the HWC.
    hwc_layer_1_t *fbt = &dc->hwLayers[1];
    fbt->handle = buffer->handle;
    fbt->blending = HWC_BLENDING_PREMULT;
    fbt->acquireFenceFd = getFenceBufferFd(buffer);
    fbt->releaseFenceFd = -1;

    sfhwc_dump_display_contents(dc);
	status = hwc->prepare(hwc, 1, &dc);
	assert(status == 0);

	sfhwc_dump_display_contents(dc);
	status = hwc->set(hwc, 1, &dc);
	assert(status == 0);

	sfhwc_dump_display_contents(dc);

	setFenceBufferFd(buffer, fbt->releaseFenceFd);
	if (fb->releaseFenceFd != -1) {
		close(fb->releaseFenceFd);
	}

	if (dc->retireFenceFd != -1) {
		close(dc->retireFenceFd);
	}

	// hwc_display_contents_1_t *dummy = (hwc_display_contents_1_t *) malloc(dcSize);
	// memset(dummy, 0, sizeof(hwc_display_contents_1_t));
	// dummy->retireFenceFd = -1;
	// dummy->flags = HWC_GEOMETRY_CHANGED;
	// sfhwc_initialize_layer(&dummy->hwLayers[0], HWC_FRAMEBUFFER_TARGET, 0, 0, m_size.x, m_size.y);
	// dummy->numHwLayers = 1;
	// sfhwc_dump_display_contents(dummy);

	// std::cout << "prepaing again..." << std::endl;
	// hwc->prepare(hwc, 1, &dummy);
	// std::cout << "prepared again..." << std::endl;

	// Question: How come if we comment out this, (forcing dc to be a unique pointer every turn)
	// the HWC locks up????
	free(dc);


	// hwc_composer_device_1_t *hwc = m_backend->hwcDevice;

	// m_hwcList->flags = HWC_GEOMETRY_CHANGED;

	// hwc_layer_1_t *fb = &m_hwcList->hwLayers[0];
	// fb->compositionType = HWC_FRAMEBUFFER;
	// fb->blending = HWC_BLENDING_NONE;
	// fb->handle = staticBuffer->handle();
	// fb->acquireFenceFd = -1;
	// fb->releaseFenceFd = -1;

	// hwc_layer_1_t *fbt = &m_hwcList->hwLayers[1];
	// fbt->compositionType = HWC_FRAMEBUFFER_TARGET;
	// fbt->blending = HWC_BLENDING_PREMULT;
	// fbt->handle = buffer->handle;
	// fbt->acquireFenceFd = getFenceBufferFd(buffer);
	// fbt->releaseFenceFd = -1;

 //    sfhwc_dump_display_contents(m_hwcList);
 //    // sfhwc_dump_hwc_device(hwc);

	// status = hwc->prepare(hwc, 1, &m_hwcList);
	// assert(status == 0);
 //    sfhwc_dump_display_contents(m_hwcList);
 //    // sfhwc_dump_hwc_device(hwc);

	// status = hwc->set(hwc, 1, &m_hwcList);
	// assert(status == 0);
 //    // sfhwc_dump_hwc_device(hwc);


	// setFenceBufferFd(buffer, fbt->releaseFenceFd);
	// fbt->releaseFenceFd = -1;

	// if (m_hwcList->retireFenceFd != -1) {
	// 	close(m_hwcList->retireFenceFd);
	// 	m_hwcList->retireFenceFd = -1;
	// }
}

RENGINE_END_NAMESPACE