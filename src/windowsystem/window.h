#pragma once

#include "../common/mathtypes.h"

namespace rengine {

class Window
{
public:
    virtual ~Window();

    void hide();
    void show();
    vec2 size() const;
};

}