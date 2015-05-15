#pragma once

#include "common/common.h"
#include "windowsystem/windowsystem.h"
#include "scenegraph/scenegraph.h"

namespace rengine {

class System
{
public:
    static System *get();
    virtual void processEvents() = 0;
    virtual Window *createWindow() = 0;
    virtual Renderer *createRenderer(Window *window) = 0;
};

}