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

#include <rengine.h>

namespace rengine_ideas
{

class Sample {
public:
    Sample(ResourceManager *resources)
    {
        setupResources(resources);
        setupObjects();
    }

    void setupResources(SurfaceInterface *iface);
    void setupObjects();

    // Resources:
    rengine::Texture *sourceTexture = nullptr;

    // Properties:
    std::string name;
    int id;

    // Signals
    Signal<> onAction;
    Signal<std::string, int> onComplexAction;

    // functions:
    void foo();
    float evaluate(const std::string &name, int id);

    // Object tree:
    rengine::RectangleNode *root = nullptr;
    rengine::TransformNode *rotation = nullptr;
    rengine::TextureNode *image = nullptr;

    // binding objects
    struct : public SignalHandler {
        void onSignal() {
            self->rotation->setDx(self->root->width() / 2.0f);
        }
        Sample *self;
    } binding_rotation_dx;
    struct : public SignalHandler {
        void onSignal() {
            self->rotation->setDy(self->root->height() / 2.0f);
        }
        Sample *self;
    } binding_rotation_dy;

    struct : public SignalHandler {
        void onSignal() {
            self->image->setX(-self->image->width() / 2.0);
        }
        Sample *self;
    } binding_image_x;
    struct : public SignalHandler {
        void onSignal() {
            self->image->setY(-self->image->height() / 2.0);
        }
        Sample *self;
    } binding_image_y;
    struct : public SignalHandler {
        void onSignal() {
            self->image->setWidth(self->textureWalker->width());
        }
        Sample *self;
    } binding_image_width;
    struct : public SignalHandler {
        void onSignal() {
            self->image->setHeight(self->textureWalker->height());
        }
        Sample *self;
    } binding_image_height;
};


/*
    Assumes something like this:
class ResourceManager
{
public:
    template <typename T> T *acquire(const char *name) { ... }
    template <typename T> release(T *) { ... }

    template <> rengine::Texture *acquire<rengine::Texture>(const char *name) { ... }
    template <> release<rengine::Texture>(rengine::Texture *texture) { ... }
};

*/

inline void Sample::setupResources(ResourceManager *resources)
{
    textureWalker = resources->acquire<rengine::Texture>("walker.png");
}

inline void setupObjects()
{
    root = rengine::RectangleNode::create();
    root->setWidth(320);
    root->setHeight(480);

    rotation = rengine::TransformNode::create();
    rotation->setRotateAroundY(1.0f);

    image = rengine::TextureNode::create();

    // Set up all signal's self references
    binding_rotation_dx.self = this;
    binding_rotation_dy.self = this;
    binding_image_x.self = this;
    binding_image_y.self = this;
    binding_image_width.self = this;
    binding_image_height.self = this;

    // Connect them
    root->widthChanged.connect(&binding_rotation_dx);
    root->heightChanged.connect(&binding_rotation_dy);
    sourceTexture.onWidthChanged.connect(&binding_image_width);
    sourceTexture.onHeightChanged.connect(&binding_image_height);
    image->onWidthChanged.connect(&binding_image_x);
    image->onHeightChanged.connect(&binding_image_y);

    // activate all signals once so values take effect.
    root->widthChanged.emit();
    root->heightChanged.emit();
    sourceTexture->onWidthChanged.emit();
    sourceTexture->onHeightChanged.emit();
    image->onWidthChanged.emit();
    image->onHeightChanged.emit();
}

}
