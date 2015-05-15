#pragma once

#include "node.h"

namespace rengine {

class Renderer
{
public:
    virtual ~Renderer() {}

    /*!
        Sets \a root as the scene root for this renderer
     */
    virtual void setSceneRoot(Node *root) = 0;

    /*!
        Renders the scene graph specified using setSceneRoot().

        Returns true if successful; false if rendering failed...
     */
    virtual bool render() = 0;
};

}