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

RENGINE_BEGIN_NAMESPACE

class Texture {
public:

    virtual ~Texture() { }

    enum Format {
        AlphaFormatMask = 0x1000,
        RGBA_32 = 1 | AlphaFormatMask,
        RGBx_32 = 2,
    };

    /*!
       The size of the surface in pixels
     */
    virtual vec2 size() const = 0;

    unsigned width() const { return size().x; }
    unsigned height() const { return size().y; }

    /*!
        Returns the format of this surface.
     */
    virtual Format format() const = 0;

    /*!
        Returns true if the surface has alpha
     */
    bool hasAlpha() const { return (format() & AlphaFormatMask) != 0; }

    /*!
        Returns the texture id of the surface
     */
    virtual GLuint textureId() const = 0;


    /*!
        A pointer to the backend that created this texture. Can be
        used as a type specifier inside the renderer/backend to
        perform downcasting.
     */
    virtual Backend *backend() const { return 0; }

};

RENGINE_END_NAMESPACE