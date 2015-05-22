#include <assert.h>
#include <iostream>

#include "rengine.h"

using namespace rengine;
using namespace std;

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
    mat4 i1;
    mat4 i2;

    {
        mat4 r = i1 * i2;
        assert(r.isIdentity());
    }

    {
        mat4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        assert(m(0, 0) == 1);
        assert(m(0, 1) == 2);
        assert(m(0, 2) == 3);
        assert(m(0, 3) == 4);
        assert(m(1, 0) == 5);
        assert(m(1, 1) == 6);
        assert(m(1, 2) == 7);
        assert(m(1, 3) == 8);
        assert(m(2, 0) == 9);
        assert(m(2, 1) == 10);
        assert(m(2, 2) == 11);
        assert(m(2, 3) == 12);
        assert(m(3, 0) == 13);
        assert(m(3, 1) == 14);
        assert(m(3, 2) == 15);
        assert(m(3, 3) == 16);
    }

    {
        mat4 m = mat4::fromTranslate(2, 3, 4);
        assert(m(0, 0) == 1);
        assert(m(0, 1) == 0);
        assert(m(0, 2) == 0);
        assert(m(0, 3) == 2);
        assert(m(1, 0) == 0);
        assert(m(1, 1) == 1);
        assert(m(1, 2) == 0);
        assert(m(1, 3) == 3);
        assert(m(2, 0) == 0);
        assert(m(2, 1) == 0);
        assert(m(2, 2) == 1);
        assert(m(2, 3) == 4);
        assert(m(3, 0) == 0);
        assert(m(3, 1) == 0);
        assert(m(3, 2) == 0);
        assert(m(3, 3) == 1);
    }

    {
        mat4 m = mat4::fromScale(2, 3, 4);
        assert(m(0, 0) == 2);
        assert(m(0, 1) == 0);
        assert(m(0, 2) == 0);
        assert(m(0, 3) == 0);
        assert(m(1, 0) == 0);
        assert(m(1, 1) == 3);
        assert(m(1, 2) == 0);
        assert(m(1, 3) == 0);
        assert(m(2, 0) == 0);
        assert(m(2, 1) == 0);
        assert(m(2, 2) == 4);
        assert(m(2, 3) == 0);
        assert(m(3, 0) == 0);
        assert(m(3, 1) == 0);
        assert(m(3, 2) == 0);
        assert(m(3, 3) == 1);
    }

    {
        mat4 m = mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
                 * mat4(21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36);
        // todo later on..
        assert(m(0, 0) == 290);
    }

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}


int main(int, char **)
{
    tst_vec2();
    tst_mat4();

    return 0;
}