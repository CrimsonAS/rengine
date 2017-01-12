/*
    Copyright (c) 2017, Gunnar Sletta <gunnar@sletta.org>
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

/*!

    The Units class contains a number of metrics which can be used by
    applications that need to relate to measurements.

    The values should be interpreted as hints that are generally sensible but
    which may not be fully accurate.

 */

class Units
{
public:
    Units(Surface *surface)
        : m_dpi(surface->dpi().y)
    {
    }

    /*!
        The base unit is a small integer value which roughly corresponds to
        the smallest readable font size, typically a few millimeters in
        physical size.

        It should be used as a base unit when writing for resolution
        independence.
     */
    float base() const { return std::round(std::max(8.0f, mm() * 2.0f)); }

    float tinyFont() const  { return base(); }
    float smallFont() const { return std::round(base() * 1.25f); }
    float font() const      { return std::round(base() * 1.75f); }
    float largeFont() const { return std::round(base() * 2.5f); }
    float hugeFont() const  { return std::round(base() * 4.0f); }

    float mm() const { return m_dpi / 25.4; }
    float cm() const { return m_dpi / 2.54; }

    float inch() const { return m_dpi; }

private:
    float m_dpi;
};

RENGINE_END_NAMESPACE