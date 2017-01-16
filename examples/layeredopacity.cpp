#include "rengine.h"

#define  STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

using namespace rengine;
using namespace std;

class MyWindow : public StandardSurface
{
public:
    Node *build() override {
        vec2 s = size();
        vec2 s3 = s / 3;

        // Root has origin in screen center
        TransformNode *root = TransformNode::create();
        root->setMatrix(mat4::translate2D(s.x * 0.5, s.y * 0.5));

        OpacityNode *opacityNode = OpacityNode::create();
        // opacityNode->setOpacity(0.5);
        root->append(opacityNode);

        vec4 color(0.5, 0.5, 0.8, 1.0);

        *opacityNode << RectangleNode::create(rect2d::fromPosSize(-s3, s3), color)
                     << RectangleNode::create(rect2d::fromPosSize(-s3/2, s3), color)
                     << RectangleNode::create(rect2d::fromPosSize(vec2(), s3), color);

        auto anim = std::make_shared<Animation_OpacityNode_opacity>(opacityNode);
        anim->setDuration(3);
        anim->setDirection(AbstractAnimation::Alternate);
        anim->setIterations(-1);
        anim->keyFrames().push_back(KeyFrame<float>(0, 0));
        anim->keyFrames().push_back(KeyFrame<float>(1, 1));

        animationManager()->start(anim);

        return root;

    }
};

RENGINE_MAIN(MyWindow)