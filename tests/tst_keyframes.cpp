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

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
*/

#include "rengine.h"
#include "test.h"

#include <vector>

struct Thing
{
    double width;
    double height;
};

struct Thing_setWidth { void operator()(double v, Thing *target) { target->width = v; } };
struct Thing_setHeight { void operator()(double v, Thing *target) { target->height = v; } };

void tst_keyframes_basic()
{
    Thing t;

    Animation animation;
    animation.setDuration(10);
    animation.setIterations(1);

    KeyFrames<Thing> keyFrames;
    keyFrames.times() << 0.0 << 0.6 << 1.0;
    keyFrames.addValues<double, Thing_setWidth>() << 100 << 200 << 300;
    keyFrames.addValues<double, Thing_setHeight>() << 11 << 22 << 33;

    animation.setRunning(true);
    for (int i=0; i<=animation.duration() * animation.iterations(); ++i) {
        animation.tick(i, &t, &keyFrames);
    }
    assert(!animation.isRunning());

    cout << __FUNCTION__ << ": ok" << endl;
}

int main(int argc, char **argv)
{

    tst_keyframes_basic();

}