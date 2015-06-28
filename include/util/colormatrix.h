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

#include <cmath>

RENGINE_BEGIN_NAMESPACE


#define RENGINE_LUMINANCE_RED   0.2125
#define RENGINE_LUMINANCE_GREEN 0.7154
#define RENGINE_LUMINANCE_BLUE  0.0721

// math based on http://www.graficaobscura.com/matrix/index.html

inline mat4 colorMatrix_hue(float radians)
{
    float mag = sqrt(2.0);
    const float xrs = 1.0 / mag;
    const float xrc = 1.0 / mag;
    mat4 mx(1, 0, 0, 0,
            0, xrc, -xrs, 0,
            0, xrs, xrc, 0,
            0, 0, 0, 1);
    const mat4 imx(1, 0, 0, 0,
                   0, xrc, xrs, 0,
                   0, -xrs, xrc, 0,
                   0, 0, 0, 1);

    mag = sqrt(3.0);
    const float yrs = -1.0/mag;
    const float yrc = sqrt(2.0)/mag;
    mat4 my(yrc, 0, yrs, 0,
            0, 1, 0, 0,
            -yrs, 0, yrc, 0,
            0, 0, 0, 1);
    const mat4 imy(yrc, 0, -yrs, 0,
                   0, 1, 0, 0,
                   yrs, 0, yrc, 0,
                   0, 0, 0, 1);

    const mat4 M = my * mx;
    const vec3 l = M * vec3(RENGINE_LUMINANCE_RED, RENGINE_LUMINANCE_GREEN, RENGINE_LUMINANCE_BLUE);
    const float zsx = l.x / l.z;
    const float zsy = l.y / l.z;
    mat4 sz(1, 0, 0, 0,
            0, 1, 0, 0,
            zsx, zsy, 1, 0,
            0, 0, 0, 1);
    mat4 isz(1, 0, 0, 0,
             0, 1, 0, 0,
             -zsx, -zsy, 1, 0,
             0, 0, 0, 1);

    const mat4 mz = mat4::rotateAroundZ(radians);


    return imx * imy * mz * isz * M * sz;
}

inline mat4 colorMatrix_brightness(float v)
{
    return mat4::scale(v, v, v);
}

inline mat4 colorMatrix_saturation(float s)
{
    const float lr = RENGINE_LUMINANCE_RED;
    const float lg = RENGINE_LUMINANCE_GREEN;
    const float lb = RENGINE_LUMINANCE_BLUE;
    const float is = 1.0f - s;
    const float ilr = is * lr;
    const float ilg = is * lg;
    const float ilb = is * lb;

    return mat4(ilr + s, ilg, ilb, 0,
                ilr, ilg + s, ilb, 0,
                ilr, ilg, ilb + s, 0,
                0, 0, 0, 1);
}

inline mat4 colorMatrix_grayscale()
{
    return colorMatrix_saturation(0);
}


RENGINE_END_NAMESPACE