#pragma once

#include <memory>

RENGINE_BEGIN_NAMESPACE

template <typename InterfaceName>
int rengine_main(int argc, char **argv) {
    std::unique_ptr<Backend> backend(Backend::get());
    InterfaceName iface;
    Surface *surface = backend->createSurface(&iface);
    surface->show();
    backend->run();
    return 0;
}

RENGINE_END_NAMESPACE

#define RENGINE_MAIN(InterfaceName)                     \
                                                        \
    RENGINE_DEFINE_BACKEND                              \
    RENGINE_ALLOCATION_POOL_NODE_DEFINITIONS            \
                                                        \
int main(int argc, char **argv) {                       \
    RENGINE_ALLOCATION_POOL(Node, 256);                 \
    RENGINE_ALLOCATION_POOL(TransformNode, 256);        \
    RENGINE_ALLOCATION_POOL(RectangleNode, 256);        \
    RENGINE_ALLOCATION_POOL(TextureNode, 256);          \
    RENGINE_ALLOCATION_POOL(OpacityNode, 64);           \
    RENGINE_ALLOCATION_POOL(ColorFilterNode, 16);       \
    RENGINE_ALLOCATION_POOL(BlurNode, 16);              \
    RENGINE_ALLOCATION_POOL(ShadowNode, 16);            \
    return RENGINE_NAMESPACE_PREFIX rengine_main<InterfaceName>(argc, argv);     \
}
