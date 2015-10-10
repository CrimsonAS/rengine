#pragma once

#include <memory>

RENGINE_BEGIN_NAMESPACE

template <typename InterfaceName>
int rengine_main(int argc, char **argv) {
    std::unique_ptr<rengine::Backend> backend(rengine::Backend::get());
    InterfaceName iface;
    rengine::Surface *surface = backend->createSurface(&iface);
    surface->show();
    backend->run();
    return 0;
}

RENGINE_END_NAMESPACE

#define RENGINE_DEFINE_GLOBALS                                                                         \
    RENGINE_BACKEND_DEFINE                                                                             \
    RENGINE_NODE_DEFINE_ALLOCATION_POOLS                                                               \
    RENGINE_NODE_DEFINE_SIGNALS                                                                        \
    RENGINE_LAYOUTNODE_DEFINE_SIGNALS                                                                  \
    RENGINE_LAYOUTNODE_DEFINE_ALLOCATION_POOLS                                                         \
                                                                                                       \


#define RENGINE_MAIN(InterfaceName)                                                                    \
    RENGINE_DEFINE_GLOBALS                                                                             \
                                                                                                       \
int main(int argc, char **argv) {                                                                      \
    RENGINE_ALLOCATION_POOL(rengine::Node, rengine_Node, 256);                                         \
    RENGINE_ALLOCATION_POOL(rengine::TransformNode, rengine_TransformNode, 256);                       \
    RENGINE_ALLOCATION_POOL(rengine::SimplifiedTransformNode, rengine_SimplifiedTransformNode, 256);   \
    RENGINE_ALLOCATION_POOL(rengine::RectangleNode, rengine_RectangleNode, 256);                       \
    RENGINE_ALLOCATION_POOL(rengine::TextureNode, rengine_TextureNode, 256);                           \
    RENGINE_ALLOCATION_POOL(rengine::OpacityNode, rengine_OpacityNode, 32);                            \
    RENGINE_ALLOCATION_POOL(rengine::ColorFilterNode, rengine_ColorFilterNode, 8);                     \
    RENGINE_ALLOCATION_POOL(rengine::BlurNode, rengine_BlurNode, 8);                                   \
    RENGINE_ALLOCATION_POOL(rengine::ShadowNode, rengine_ShadowNode, 8);                               \
    return RENGINE_NAMESPACE_PREFIX rengine_main<InterfaceName>(argc, argv);                           \
}
