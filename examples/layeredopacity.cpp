#include "rengine.h"

using namespace rengine;
using namespace std;

class MyWindow : public StandardSurface
{
public:
    Node *update(Node *old) {
        if (old)
            return old;

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

        AnimationClosure<OpacityNode> *anim = new AnimationClosure<OpacityNode>(opacityNode);
        anim->setDuration(3);
        anim->setDirection(Animation::Alternate);
        anim->setIterations(-1);
        anim->keyFrames.times() << 0 << 1;
        anim->keyFrames.addValues<double, OpacityNode_setOpacity>() << 0 << 1;
        animationManager()->startAnimation(anim);

        return root;

    }
};

RENGINE_MAIN(MyWindow)