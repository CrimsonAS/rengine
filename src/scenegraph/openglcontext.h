#pragma once

namespace rengine
{

class OpenGLContext
{
public:
    virtual bool makeCurrent(Surface *surface) = 0;
    virtual bool swapBuffers(Surface *surface) = 0;
};

}