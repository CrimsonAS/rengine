#include <rengine.h>

namespace rengine_ideas
{

class Sample {
public:
    Sample(Renderer *renderer)
    {
        setupResources(renderer);
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

inline void Sample::setupResources(Renderer *renderer)
{
    textureWalker = loadTexture(renderer, "walker.png");
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
    rengine::RectangleNode::widthChanged.connect(root, &binding_rotation_dx);
    rengine::RectangleNode::heightChanged.connect(root, &binding_rotation_dy);
    rengine::Texture::onWidthChanged.connect(texture, &binding_image_width);
    rengine::Texture::onHeightChanged.connect(texture, &binding_image_height);
    rengine::TextureNode::onWidthChanged.connect(image, &binding_image_x);
    rengine::TextureNode::onHeightChanged.connect(image, &binding_image_y);

    // activate all signals once so values take effect.
    rengine::RectangleNode::widthChanged.emit(root);
    rengine::RectangleNode::heightChanged.emit(root);
    rengine::Texture::onWidthChanged.emit(texture);
    rengine::Texture::onHeightChanged.emit(texture);
    rengine::TextureNode::onWidthChanged.emit(image);
    rengine::TextureNode::onHeightChanged.emit(image);
}

}