/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*!

    This file is fully self-contained and can be deployed in other projects
    with minimal additional dependencies. Just define RENGINE_BEGIN_NAMESPACE
    and RENGINE_END_NAMESPACE before including it, and you should be good.

 */

#pragma once

RENGINE_BEGIN_NAMESPACE

template <typename Value>
class KeyFrame
{
public:
    KeyFrame(double time, Value value)
        : m_time(time)
        , m_value(value)
    {
    }

    double time() const { return m_time; }
    void setTime(double t) { m_time = t; }

    Value value() const { return m_value; }
    void setValue(const Value &value) { m_value = value; }

private:
    double m_time;
    Value m_value;
};


class AnimationCurves
{
public:
    static double linear(double t) { return t; }
    static double smooth(double t) { return 3*t*t - 2*t*t*t; }
};


/*!

    Defines an animation, greatly inspored by the HTML/CSS animation
    primitives.

    The animation contains a few built-in properties, such as iterations,
    direction and duration which are specified through normal functions.

 */
class AbstractAnimation : public SignalEmitter
{
public:
    virtual ~AbstractAnimation() {}

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

    virtual void tick(double time) = 0;

    /*!
        Contains the length of each iteration in seconds.

        The total running time of the animation will be iterations() *
        duration().

        It is an error to change the duration once the animation is running.

        By default, the duration is 1 second.
     */
    double duration() const { return m_duration; }
    void setDuration(double dur) {
        assert(!isRunning());
        m_duration = dur;
    }

    /*!
        Contains the number of iterations this animation should run before
        stopping, where -1 signifies an infinite amount of iterations.

        The total running time of the animation will be iterations() *
        duration().

        It is an error to change the number of iterations once the animation
        is running.

        by default, the number of iterations is 1.
     */
    int iterations() { return m_iterations; }
    void setIterations(int iterations) {
        assert(!isRunning());
        m_iterations = iterations;
    }

    /*!
        Sets the direciton the animation should be running. The tick()
        function should be increment normally from 0 to iterations() *
        duration() regardless of type of direction.

        It is an error to change the direction once the animation is running.

        By default, the direction is Normal.
     */

    Direction direction() const { return (Direction) m_direction; }
    void setDirection(Direction d) {
        assert(!isRunning());
        m_direction = d;
    }

    bool isRunning() const { return m_running; }
    void setRunning(bool r) { m_running = r; }

protected:
    int m_iterations;
    double m_duration;

    unsigned m_running : 1;
    unsigned m_direction : 2;
};


template <typename Target,
          typename Value,
          void (Target::*MemberFunction)(Value),
          double (*Curve)(double) = &AnimationCurves::smooth>
class Animation : public AbstractAnimation
{
public:
    Animation(Target *target) : m_target(target) { }

    std::vector<KeyFrame<Value>> &keyFrames() { return m_keyFrames; }

    void tick(double time)
    {
        assert(isRunning());
        assert(m_keyFrames.size()); // no animations without 'em.

        // std::cout << "tick: time=" << time << std::endl;
        bool stop = false;
        int curIter = (int) (time / m_duration);
        double timeInThisIteration = fmod(time, m_duration);
        if (m_iterations >= 0 && curIter >= m_iterations) {
            stop = true;
            timeInThisIteration = m_duration; // so we run until the very end..
            curIter = m_iterations - 1; // set to last iteration
            // std::cout << " - reached the end" << std::endl;
        }

        double scaledTime = timeInThisIteration / m_duration;

        // reverse direction if applicable
        if (m_direction == Reverse
            || (m_direction == Alternate && ((curIter % 2) == 1))
            || (m_direction == AlternateReverse && (curIter % 2 == 0))) {
            // std::cout << " - reversed..." << std::endl;
            scaledTime = 1.0 - scaledTime;
        }

        // Find the two indices to interpolate between
        size_t i0 = 0;
        size_t i1 = 1;
        // we're before the thing starts...
        if (m_keyFrames.front().time() > scaledTime) {
            i0 = i1 = 0;
        } else {
            while (i1 < m_keyFrames.size() && m_keyFrames.at(i1).time() < scaledTime)
                ++i1;
            // if (kft.at(i1) > scaledTime) // common case
            if (i1 >= m_keyFrames.size())
                i0 = i1 = m_keyFrames.size() - 1;
            else
                i0 = i1 - 1;
        }

        // std::cout << " - using: [ " << i0 << " -> " << i1 << " ] out of " << m_keyFrames.size() << std::endl;

        assert(i0 >= 0);
        assert(i1 < m_keyFrames.size());
        double t0 = m_keyFrames.at(i0).time();
        double t1 = m_keyFrames.at(i1).time();
        double t = (i0 < i1) ? (scaledTime - t0) / (t1 - t0) : t1;
        double et = Curve(t);

        // std::cout << " - time: [ " << t0 << " -> " << t1 << " ] t=" << t << "; eased=" << et << "; st=" << scaledTime << std::endl;

        Value v0 = m_keyFrames.at(i0).value();
        Value v1 = m_keyFrames.at(i1).value();
        Value value = v0 * (1.0 - et) + v1 * et;
        (m_target->*MemberFunction)(value);

        if (stop)
            setRunning(false);
    }


private:
    Target *m_target;
    std::vector<KeyFrame<Value>> m_keyFrames;
};


typedef std::chrono::steady_clock clock;
typedef std::chrono::steady_clock::time_point time_point;

class AnimationManager : public SignalEmitter
{
public:
    AnimationManager()
    {
    }

    static Signal<> onRunningChanged;

    void tick();
    void start(const std::shared_ptr<AbstractAnimation> &animation, double delay = 0.0);
    void stop(const std::shared_ptr<AbstractAnimation> &animation);

    bool animationsRunning() const { return !m_runningAnimations.empty(); }
    bool animationsScheduled() const { return !m_scheduledAnimations.empty(); }

    bool isRunning() const { return m_running; }

private:
    time_point now();
    void setRunning(bool running);

    struct ManagedAnimation {
        time_point start;
        std::shared_ptr<AbstractAnimation> animation;
        std::vector<std::shared_ptr<AbstractAnimation>> animations;
        int index = 0;
    };

    time_point m_nextTick;

    std::list<ManagedAnimation> m_runningAnimations;
    std::list<ManagedAnimation> m_scheduledAnimations;

    bool m_running;
};

time_point AnimationManager::now()
{
    if (!m_running)
        m_nextTick = clock::now();
    return m_nextTick;
}

void AnimationManager::setRunning(bool running)
{
    if (running == m_running)
        return;
    m_running = running;
    onRunningChanged.emit(this);
}

void AnimationManager::start(const std::shared_ptr<AbstractAnimation> &animation, double delay)
{
    setRunning(true);
    ManagedAnimation m;
    m.start = now() + std::chrono::microseconds(int64_t(delay * 1000000));
    m.animation = animation;
    m_scheduledAnimations.push_back(m);
}

void AnimationManager::stop(const std::shared_ptr<AbstractAnimation> &animation)
{
    for (auto it = m_runningAnimations.begin(); it != m_runningAnimations.end(); ++it) {
        if (it->animation == animation) {
            assert(animation->isRunning());
            m_runningAnimations.erase(it);
            break;
        }
    }
    for (auto it = m_scheduledAnimations.begin(); it != m_scheduledAnimations.end(); ++it) {
        if (it->animation == animation) {
            m_scheduledAnimations.erase(it);
            break;
        }
    }
}

void AnimationManager::tick()
{
    // alternatively:
    // time_point now = clock::now();
    time_point now = m_nextTick;
    m_nextTick += std::chrono::milliseconds(16);

    // std::cout << "AnimationManager::tick: scheduled=" << m_scheduledAnimations.size()
    //           << ", running=" << m_runningAnimations.size() << std::endl;

    // Start pending animations if we've passed beyond its starting point.
    // ### TODO: these should probably be sorted so animations start in the right order
    auto si = m_scheduledAnimations.begin();
    while (si != m_scheduledAnimations.end()) {
        assert(!si->animation->isRunning());
        if (si->start < now) {
            // Make sure we start at t=0
            si->start = now;
            si->animation->setRunning(true);
            AbstractAnimation::onStarted.emit(si->animation.get());
            m_runningAnimations.push_back(*si);
            si = m_scheduledAnimations.erase(si);
        } else {
            ++si;
        }
    }

    auto ri = m_runningAnimations.begin();
    while (ri != m_runningAnimations.end()) {
        assert(ri->animation->isRunning());
        std::chrono::duration<double> animTime = now - ri->start;
        ri->animation->tick(animTime.count());
        if (!ri->animation->isRunning()) {
            AbstractAnimation::onCompleted.emit(ri->animation.get());
            ri = m_runningAnimations.erase(ri);
        } else {
            ++ri;
        }
    }

    if (m_scheduledAnimations.size() == 0 && m_runningAnimations.size() == 0)
        setRunning(false);
}


#define RENGINE_DEFINE_ANIMATION_SIGNALS                            \
    rengine::Signal<> rengine::AbstractAnimation::onStarted;        \
    rengine::Signal<> rengine::AbstractAnimation::onCompleted;      \
    rengine::Signal<> rengine::AnimationManager::onRunningChanged;  \

RENGINE_END_NAMESPACE
