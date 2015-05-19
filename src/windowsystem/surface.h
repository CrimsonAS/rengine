#pragma once

#include "../common/mathtypes.h"

namespace rengine {

class Surface
{
public:
    virtual ~Surface() {}

    virtual void hide() = 0;
    virtual void show() = 0;
    virtual vec2 size() const = 0;
};

}