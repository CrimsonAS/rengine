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

SfHwcBuffer::SfHwcBuffer(SfHwcBackend *backend, int w, int h, int format, int usage)
	: m_backend(backend)
	, m_handle(0)
	, m_bits(0)
	, m_stride(0)
	, m_width(w)
	, m_height(w)
{
	if (w == 0 || h == 0)
		return;

    if (allocDevice()->alloc(allocDevice(), w, h, format, usage, &m_handle, &m_stride) || !m_handle) {
    	logw << "allocation failed, size=" << w << "x" << h << ", "
    		 << "format=" << std::hex << format << ", usage=" << usage << std::dec
    		 << std::endl;
    	return;
    }

    m_width = w;
    m_height = h;

    logd << "created buffer, size=" << w << "x" << h << ", format=" << std::hex << format << ", usage=" << usage << std::dec << std::endl;
}

SfHwcBuffer::~SfHwcBuffer()
{
	release();
}

void SfHwcBuffer::lock()
{
	if (!m_handle)
		return;
	if (grallocModule()->lock(grallocModule(), m_handle, GRALLOC_USAGE_SW_WRITE_RARELY, 0, 0, m_width, m_height, (void **) &m_bits) != 0 || !m_bits) {
    	logw << "failed..." << std::endl;
    	m_bits = 0;
    }
}

void SfHwcBuffer::unlock()
{
	if (!m_handle)
		return;
	m_bits = 0;
	if (grallocModule()->unlock(grallocModule(), m_handle) != 0)
    	logw << "failed..." << std::endl;
}

void SfHwcBuffer::release()
{
	if (!m_handle)
		return;
	allocDevice()->free(allocDevice(), m_handle);
	m_handle = 0;
	m_bits = 0;
	m_stride = 0;
	m_width = 0;
	m_height = 0;
}

void SfHwcBuffer::fillWithCrap()
{
	assert(isLocked());

    unsigned r = rand() % 256;
    unsigned g = rand() % 256;
    unsigned b = rand() % 256;

    float cy = (rand() % 100) / 100.0;
    float cx = (rand() % 100) / 100.0;
    int iter = 50;

    int w = m_width;
    int h = m_height;

    unsigned *bits = (unsigned *) m_bits;

    for (int iy=0; iy<h; ++iy) {
        float ty = iy / (float) h;
        for (int ix=0; ix<w; ++ix) {
            float tx = ix / (float) w;

            float zx = 2.0 * (tx - 0.5);
            float zy = 3.0 * (ty - 0.5);
            int i = 0;
            for (i=0; i<iter; ++i) {
                float x = (zx * zx - zy * zy) + cx;
                float y = (zy * zx + zx * zy) + cy;

                if (x*x + y*y > 4.0) break;
                zx = x;
                zy = y;
            }

            float fv = i / float(iter);
            if (fv >= 1.0f)
                fv = 0.0f;
            fv = fv * 3.0f;

            unsigned pa = std::min<unsigned>(255, (fv * 255));
            unsigned pr = std::min<unsigned>(255, (fv * r));
            unsigned pg = std::min<unsigned>(255, (fv * g));
            unsigned pb = std::min<unsigned>(255, (fv * b));
            bits[ix + iy * m_stride] = (pa << 24) | (pb << 16) | (pg << 8) | (pr);
        }
    }
}

RENGINE_END_NAMESPACE