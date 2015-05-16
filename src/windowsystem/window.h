#pragma once

#include "../common/mathtypes.h"

namespace rengine {

class Window
{
public:
    virtual ~Window() {}

    virtual void hide() = 0;
    virtual void show() = 0;
    virtual vec2 size() const = 0;
};

}