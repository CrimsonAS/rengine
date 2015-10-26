/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
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

#include <map>


// ### I would prefer that instead of pulling this in here, we would pull this
// into the application in a subclass of ResourceManager instead, but then
// ResourceManager becomes really hard to use outside, so I don't have a better
// sollution atm.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

RENGINE_BEGIN_NAMESPACE

class ResourceManager
{
public:
    virtual ~ResourceManager() {
        for (auto i : m_textures) {
            if (i.second.refCount != 0) {
                logw << "texture resource, key=" << i.first << ", has refCount="
                     << i.second.refCount << " when resource manager is destroyed" << std::endl;
            }
            delete i.second.texture;
        }
    }

    template <typename T> T *acquire(const std::string &key) { assert(0); return 0; }
    template <typename T> void release(T *t) { assert(0); }

    void setRenderer(Renderer *renderer) { m_renderer = renderer; }
    Renderer *renderer() const { return m_renderer; }

    virtual Texture *onLoadTexture(const std::string &key);

protected:
    Renderer *m_renderer = nullptr;

    struct TrackedTexture {
        int refCount;
        Texture *texture;
    };

    std::map<std::string, TrackedTexture> m_textures;
};

Texture *ResourceManager::onLoadTexture(const std::string &key)
{
    int w, h, n;
    logd << "loading image: " << key << std::endl;
    unsigned char *data = stbi_load(key.c_str(), &w, &h, &n, 4);

    if (!data) {
        logw << "Failed to load image '" << key << "'.." << std::endl;
        return 0;
    }

    logd << " -> " << key << ": size=" << w << "x" << h << ", components=" << n << std::endl;
    // Premultiply it...
    if (n == 4) {
        for (int y=0; y<h; ++y) {
            for (int x=0; x<w; ++x) {
                unsigned char *p = data + (y * w + x) * 4;
                unsigned a = p[3];
                p[0] = (unsigned(p[0]) * a) / 255;
                p[1] = (unsigned(p[1]) * a) / 255;
                p[2] = (unsigned(p[2]) * a) / 255;
            }
        }
        logd << " -> premultiplied" << std::endl;
    }

    assert(m_renderer);
    Texture *texture = m_renderer->createTextureFromImageData(vec2(w,h), Texture::RGBA_32, data);
    logd << " -> texture=" << texture << std::endl;
    assert(texture);
    STBI_FREE(data);
    return texture;
}

template <> Texture *ResourceManager::acquire<Texture>(const std::string &key)
{
    logd << "key=" << key << std::endl;

    auto texIt = m_textures.find(key);
    if (texIt != m_textures.end()) {
        texIt->second.refCount++;
        return texIt->second.texture;
    }

    logd << " -> " << key << " is not in cache, calling onLoadTexture" << std::endl;
    Texture *texture = onLoadTexture(key);
    if (texture != 0) {
        logd << " -> " << key << "=" << texture << std::endl;
        TrackedTexture tt;
        tt.refCount = 1;
        tt.texture = texture;
        m_textures[key] = tt;
        return texture;
    }

    logw << "Failed to acquire texture for key='" << key << "'" << std::endl;

    return 0;
}

RENGINE_END_NAMESPACE