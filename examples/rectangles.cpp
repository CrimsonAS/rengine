#include "rengine.h"
#include "examples.h"

class MyWindow : public StandardSurfaceInterface
{
public:
    MyWindow()
        : m_layer(0)
    {
    }

    ~MyWindow()
    {
        delete m_layer;
    }

    Node *update(Node *old) {
        if (old)
            return old;

        vec2 s = surface()->size();

        m_layer = rengine_loadImage(renderer(), "walker.png");

        // Root has origin in screen center
        TransformNode *root = new TransformNode();
        root->setMatrix(mat4::translate2D(s.x * 0.5, s.y * 0.5));

        TransformNode *xnode = new TransformNode();
        xnode->setProjectionDepth(1000);
        xnode->setMatrix(mat4::rotateAroundX(-0.15));

        TransformNode *rotationNode = new TransformNode();
        xnode->append(rotationNode);

        const int count = 20;
        for (int i=0; i<count; ++i) {
            float c = i / float(count - 1);
            float w = s.x * 0.1;
            float h = s.y * 0.2;

            TransformNode *tn = new TransformNode();

            tn->setMatrix(mat4::rotateAroundY(M_PI * 2.0 * (i / float(count)))
                          * mat4::translate(s.x * 0.33, 0, 0)
                          * mat4::rotateAroundY(M_PI/2.0));

            *tn << new RectangleNode(rect2d::fromXywh(-w/2, -h/2, w, h), vec4(c, 0.5, 1-c, 0.8));
            rotationNode->append(tn);

            TransformNode *depthAdjustment = new TransformNode();
            depthAdjustment->setMatrix(mat4::rotateAroundY(M_PI * 2.0 * (i / float(count)))
                                       * mat4::translate(s.x * 0.33 + 2, 0, 0)
                                       * mat4::rotateAroundY(M_PI/2.0));
            *depthAdjustment << new LayerNode(rect2d::fromXywh(-w/2, -h/2, w, h), m_layer);
            rotationNode->append(depthAdjustment);
        }

        root->append(xnode);

        AnimationClosure<TransformNode, SmoothedTimingFunction> *anim = new AnimationClosure<TransformNode, SmoothedTimingFunction>(rotationNode);
        anim->setDuration(4);
        anim->setIterations(-1);
        anim->keyFrames.times() << 0 << 1;
        anim->keyFrames.addValues<double, TransformNode_rotateAroundY>() << 0 << M_PI * 2.0;
        animationManager()->startAnimation(anim);

        return root;
    }

private:
    Layer *m_layer;
};

RENGINE_MAIN(MyWindow)