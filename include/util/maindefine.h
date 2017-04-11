/*
 * Copyright (c) 2017 Crimson AS <info@crimson.no>
 * Author: Gunnar Sletta <gunnar@crimson.no>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <memory>

RENGINE_BEGIN_NAMESPACE

template <typename SurfaceName>
int rengine_main(int argc, char **argv) {
    RENGINE_BACKEND backend;
    SurfaceName surface;
    surface.show();
    backend.run();
    return 0;
}

RENGINE_END_NAMESPACE

#define RENGINE_DEFINE_GLOBALS                                                                         \
    RENGINE_BACKEND_DEFINE                                                                             \
    RENGINE_SIGNALEMITTER_DEFINE_SIGNALS                                                               \
    RENGINE_NODE_DEFINE_ALLOCATION_POOLS                                                               \
    RENGINE_NODE_DEFINE_SIGNALS                                                                        \
    RENGINE_LAYOUTNODE_DEFINE_SIGNALS                                                                  \
    RENGINE_LAYOUTNODE_DEFINE_ALLOCATION_POOLS                                                         \
    RENGINE_DEFINE_ANIMATION_SIGNALS                                                                   \


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
