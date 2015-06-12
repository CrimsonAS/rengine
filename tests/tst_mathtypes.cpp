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

#include <iostream>

#include "rengine.h"
#include "test.h"

void tst_vec2()
{
    vec2 a(1, 2);
    vec2 b(3, 4);

    vec2 r = a + b;
    assert(r.x == 4);
    assert(r.y == 6);

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

void tst_mat4()
{
    {
        mat4 r = mat4() * mat4();
        assert(r.isIdentity());
    }

    {
        mat4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        check_equal(m(0, 0), 1);
        check_equal(m(0, 1), 2);
        check_equal(m(0, 2), 3);
        check_equal(m(0, 3), 4);
        check_equal(m(1, 0), 5);
        check_equal(m(1, 1), 6);
        check_equal(m(1, 2), 7);
        check_equal(m(1, 3), 8);
        check_equal(m(2, 0), 9);
        check_equal(m(2, 1), 10);
        check_equal(m(2, 2), 11);
        check_equal(m(2, 3), 12);
        check_equal(m(3, 0), 13);
        check_equal(m(3, 1), 14);
        check_equal(m(3, 2), 15);
        check_equal(m(3, 3), 16);
    }

    {
        mat4 m = mat4::translate2D(2, 3);
        check_true(m.type <= mat4::Translation2D);
        check_equal(m, mat4(1, 0, 0, 2,
                            0, 1, 0, 3,
                            0, 0, 1, 0,
                            0, 0, 0, 1));
    }

    {
        mat4 m = mat4::scale2D(2, 3);
        check_true(m.type <= mat4::Scale2D);
        check_equal(m, mat4(2, 0, 0, 0,
                            0, 3, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1));
    }

    {
        mat4 m = mat4( 1,  2,  3,  4,
                       5,  6,  7,  8,
                       9, 10, 11, 12,
                      13, 14, 15, 16)
                 *
                 mat4(21, 22, 23, 24,
                      25, 26, 27, 28,
                      29, 30, 31, 32,
                      33, 34, 35, 36);
        check_true(m.type == mat4::Generic);
        check_equal(m, mat4( 290,  300,  310,  320,
                             722,  748,  774,  800,
                            1154, 1196, 1238, 1280,
                            1586, 1644, 1702, 1760));

    }

    { // translate
        mat4 m = mat4::translate2D(1, 2) * mat4::translate2D(10, 20);
        check_true(m.type == mat4::Translation2D);
        check_equal(m, mat4(1, 0, 0, 11,
                            0, 1, 0, 22,
                            0, 0, 1, 0,
                            0, 0, 0, 1));
    }

    { // scale
        mat4 m = mat4::scale2D(1, 2) * mat4::scale2D(3, 4);
        check_equal(m.type, mat4::Scale2D);
        check_equal(m, mat4(3, 0, 0, 0,
                            0, 8, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1));
    }

    { // Rotate...
        mat4 m = mat4::rotate2D(M_PI / 2.0f);
        check_true(m.type == mat4::Rotation2D);
        check_fuzzyEqual(m(0,0), 0);
        check_fuzzyEqual(m(0,1), -1);
        check_equal(m(0,2), 0.0f);
        check_equal(m(0,3), 0.0f);

        check_fuzzyEqual(m(1,0), 1);
        check_fuzzyEqual(m(1,1), 0);
        check_equal(m(1,2), 0.0f);
        check_equal(m(1,3), 0.0f);

        check_equal(m(2,0), 0.0f);
        check_equal(m(2,1), 0.0f);
        check_equal(m(2,2), 1.0f);
        check_equal(m(2,3), 0.0f);

        check_equal(m(3,0), 0.0f);
        check_equal(m(3,1), 0.0f);
        check_equal(m(3,2), 0.0f);
        check_equal(m(3,3), 1.0f);
    }

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

void tst_mat4_vecx()
{
    { // translate2D
        mat4 t = mat4::translate2D(10, 20);
        check_equal(t * vec2(1, 2),       vec2(11, 22));
        check_equal(t * vec3(1, 2, 3),    vec3(11, 22, 3));
        check_equal(t * vec4(1, 2, 3, 4), vec4(41, 82, 3, 4));
    }

    { // scale2D
        mat4 t = mat4::scale2D(10, 20);
        check_equal(t * vec2(1, 2),       vec2(10, 40));
        check_equal(t * vec3(1, 2, 3),    vec3(10, 40, 3));
        check_equal(t * vec4(1, 2, 3, 4), vec4(10, 40, 3, 4));
    }

    { // rotated2D
        mat4 t = mat4::rotate2D(M_PI/2);
        check_fuzzyEqual(t * vec2(1, 2),       vec2(-2, 1));
        check_fuzzyEqual(t * vec3(1, 2, 3),    vec3(-2, 1, 3));
        check_fuzzyEqual(t * vec4(1, 2, 3, 4), vec4(-2, 1, 3, 4));
    }

    {
        mat4 m = mat4::scale2D(2, 3) * mat4::rotate2D(M_PI/2.0) * mat4::translate2D(4, 5);
        vec3 v3 = m * vec3(10, 20, 30);
        check_fuzzyEqual(v3.x, -50);
        check_fuzzyEqual(v3.y, 42);
        check_fuzzyEqual(v3.z, 30);

        vec2 v2 = m * vec2(10, 20);
        check_fuzzyEqual(v2.x, -50);
        check_fuzzyEqual(v2.y, 42);
    }

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}


int main(int, char **)
{
    tst_vec2();
    tst_mat4();
    tst_mat4_vecx();

    return 0;
}