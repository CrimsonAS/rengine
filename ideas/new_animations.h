
template <typename Value>
class KeyFrame
{
public:
    KeyFrame(double time, Value value)
        : m_time(time)
        : m_value(value);

    double time() const { return m_time; }
    void setTime(double t) { m_time = t; }

    struct NoValue { };

private:
    double m_time;
    Value m_value;
};

NoValue operator*(double, NoValue) { return NoValue(); }
NoValue operator+(double, NoValue) { return NoValue(); }

class AbstractAnimation : public SignalEmitter
{
public:
    enum Direction { // uses 2-bit field
        Normal,
        Reverse,
        Alternate,
        AlternateReverse
    };

    AbstractAnimation()
      : m_iterations(1)
      , m_duration(1)
      , m_running(false)
      , m_direction(Normal)
    {
    }

    static Signal<> onStarted;
    static Signal<> onCompleted;

    virtual void tick(double /*time*/) = 0;

    double duration() const { return m_duration; }
    void setDuration(double dur) {
        assert(!isRunning());
        m_duration = dur;
    }

    int iterations() { return m_iterations; }
    void setIterations(int iterations) {
        assert(!isRunning());
        m_iterations = iterations;
    }

    Direction direction() const { return (Direction) m_direction; }
    void setDirection(Direction d) {
        assert(!isRunning());
        m_direction = d;
    }

    bool isRunning() const { return m_running; }
    void setRunning(bool r) { m_running = r; }

private:
    int m_iterations;
    double m_duration;

    unsigned m_running : 1;
    unsigned m_direction : 2;
};

template <typename Target, void (Target::*MemberFunction)(), typename Value, double (*SmoothingFunction)(double) = &Animation::linearSmoothingFunction>
class Animation : public AbstractAnimation
{
public:
    Animation(Target *target) : m_target(target) { }

    std::vector &keyFrames() { return m_keyFrames; }

    void tick(double time) {
        double smoothedTime = SmoothingFunction(time);
    }

    // Smoothing functions
    static double smooth(double t) { return 3*t*t - 2*t*t*t; }
    static double linear(double t) { return t; }


private:
    Target *m_target;
    std::vector<KeyFrame<Values...>> m_keyFrames;
};


class AnimationManager
{
public:
    void start(const std::shared_ptr<AbstractAnimation> &animation, double delay = 0.0f);             // starts a single animation, potentially at a later time..
    void start(const std::vector<std::shared_ptr<AbstractAnimation>> &chain, double delay = 0.0f);    // starts a chain of animations
};

// Example of convenience stuff

typedef Animation<OpacityNode, &OpacityNode::setOpacity, float, &Animation::smooth> Animation_OpacityNode_opacity;

// These functions don't exist yet, because the TransformNode is just a bare
// minimum atm. Adding these to do the same as our current animation appliers
// do is trivial though. The reason this wasn't originally done is because
// they write the matrix directly.
//
// SimpleTransformNode stores separate properties for each rotation / scale
// factor and uses those to build the matrix based on running preprocess.
//

// With an animation system like this, if one wanted to animation multiple
// properties concurrently, that could now either be done by using the
// SimpleTransformNode which is property-based, or we could use a KeyFrame
// Value type which has all of them, give that value an interpolation function
// (* and + operators) and then implement a function which applies that
// keyframe value.
typedef Animation<TransformNode, &TransformNode::setRotation, float, &Animation::smooth> Animation_TransformNode_rotation;
typedef Animation<TransformNode, &TransformNode::setRotationAroundZ, float, &Animation::smooth> Animation_TransformNode_rotationAroundZ;
typedef Animation<TransformNode, &TransformNode::setRotationAroundY, float, &Animation::smooth> Animation_TransformNode_rotationAroundY;
typedef Animation<TransformNode, &TransformNode::setRotationAroundX, float, &Animation::smooth> Animation_TransformNode_rotationAroundX;

// Example of use:
inline void Foo::somewhere()
{
    OpacityNode *opacityNode = OpacityNode::create();
    // ...

    // Set up animation of opacity..
    auto animation = make_shared<Animation_OpacityNode_opacity>(opacityNode);
    animation->setDuration(3);                          // 3 seconds
    animation->setDuration(Animation::Alternate);       // Alternating, fade in and out
    animation->setIterations(5);                        // 5 times total
    animation->keyFrames() << KeyFrame(0, 1)            // opacity = 1 at the start
                           << KeyFrame(1, 0);           // opacity = 0 at the end..
    animationManager()->start(animation);

    // Set up a singleshot
    auto singleShot = make_shared<Animation<Foo, &Foo::doSomething, KeyFrame::NoValue, &Animation::linear>>();
    animationManager->schedule(10, singleShot);

    // Set up chain of animations. These are executed one after the other.
    auto z = make_shared<Animation_TransformNode_rotateAroundZ>(xform);
    auto y = make_shared<Animation_TransformNode_rotateAroundY>(xform);

    z->keyFrames() << KeyFrame(0, 0) << KeyFrame(1, M_PI * 2); // rotate
    y->keyFrames() << KeyFrame(0, 0) << KeyFrame(1, M_PI * 2); // rotate

    std::vector<std::shared_ptr<AbstractAnimation>> chain;
    chain.push_back(z);
    chain.push_back(y);
    chain.push_back(singleShot);
    animationManager->start(chain);
}


