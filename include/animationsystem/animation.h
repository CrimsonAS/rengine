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

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
*/

/*!

    This file is fully self-contained and can be deployed in other projects
    with minimal additional dependencies. Just define RENGINE_BEGIN_NAMESPACE
    and RENGINE_END_NAMESPACE before including it, and you should be good.

 */


#pragma once

#include <vector>

RENGINE_BEGIN_NAMESPACE

/*!
    The default timing function used together with Animation.

    Implements a linear timing function.
 */
class LinearTimingFunction
{
public:
    double operator()(double t) { return t; }
};

/*!

    Defines an animation, greatly inspored by the HTML/CSS animation
    primitives.

    The animation contains a few built-in properties, such as iterations,
    direction and duration which are specified through normal functions.

    The values the animation operates on is specified by the keyFrames objects
    which contians information about the keyframes. The animation is
    progressed by calling the tick() function and by incrementing its time.
    The result of a call to tick() is that the animation object will
    interpolate between the values in the keyframes object and call back
    through the KeyFrameValue's ApplyFunctor so the appliction can pick up the
    value.

    An easing curve can be applied between each keyframe so that animations
    accellerates and decellerates according to application design. This is
    done by specifying a TimingFunction template argument. The default is a
    linear curve.

 */
template <typename Target,
          typename TimingFunction = LinearTimingFunction>
class Animation
{
public:
    enum Direction { // uses 2-bit field
        Normal,
        Reverse,
        Alternate,
        AlternateReverse
    };

    Animation()
      : m_keyFrames(0)
      , m_target(0)
      , m_iterations(1)
      , m_duration(1)
      , m_running(false)
      , m_direction(Normal)
    {
    }

    inline void tick(double time);

    /*!
        Returns the target for this animation. The target is
        passed to the KeyFrameValues's apply functor.

        It is an error to change the target once the animation is running.

        By default, the target is 0.
     */
    Target *target() const { return m_target; }
    void setTarget(Target *t) {
        assert(!isRunning());
        m_target = t;
    }

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

    /*!
        Specifies the keyframes to use for this animation.

        It is an error to change the keyframes once the animation is running.

        The Animation does not take ownership over the KeyFrames object.
     */
    KeyFrames<Target> *keyFrames() const { return m_keyFrames; }
    void setKeyFrames(KeyFrames<Target> *kf);

    /*!
        Returns a pointer to the timimg function used by this animation.
     */
    TimingFunction *timingFunction() { return m_timingFunction; }

private:
    KeyFrames<Target> *m_keyFrames;
    Target *m_target;
    TimingFunction m_timingFunction;
    int m_iterations;
    double m_duration;

    unsigned m_running : 1;
    unsigned m_direction : 2;
};

/*!
    Abstract base class for specifying keyframe values; in code use
    KeyFrameValues<ValueType, Target, ApplyFunctor>.
 */
template <typename Target>
class KeyFrameValuesBase
{
public:
    virtual ~KeyFrameValuesBase() {}
    virtual void interpolate(int i0, int i1, double v, Target *target) = 0;
    virtual int size() const = 0;
};

template <typename ValueType, typename Target, typename ApplyFunctor>
class KeyFrameValues : public KeyFrameValuesBase<Target>
{
public:
    void interpolate(int i0, int i1, double t, Target *target) {
        assert(i0 >= 0);
        assert(i0 < values.size());
        assert(i1 > 0);
        assert(i1 < values.size());
        assert(t >= 0);
        assert(t <= 1);
        const ValueType &v0 = values.at(i0);
        const ValueType &v1 = values.at(i1);
        applyFunctor(v0 + (v1 - v0) * t, target);
    }

    /*!
        Add \a value to this keyframevalues set
     */
    void append(const ValueType &value) { values.push_back(value); }

    /*!
        Convenience overload to append() which allows for appending
        multiple values in one go.
     */
    KeyFrameValues<ValueType, Target, ApplyFunctor> &operator<<(const ValueType &v) { append(v); return *this; }

    int size() const { return values.size(); }

private:
    ApplyFunctor applyFunctor;
    std::vector<ValueType> values;
};

/*!

    The keyframe object defines a set of values at specific points in time.
    The Animation class uses this as input and interpolates between these
    values over time so that what comes out is a smooth animation between key
    points in time.

    Times are relative, ranging from 0 to 1. It is required that one first
    defines the number of times, then adds values to be interpolated. The
    number of entries in the values and the times must be identical.

    Here is an example on how to define a KeyFrames object.

    \code
    struct MyThing {
        ...
        void setWidth(double width);
        int width() const;
    }

    struct MyThing_setWidth {
        void operator()(double v, MtThing *target) { target->width = v; }
    };

    ...

    MyThing thing;

    KeyFrames<MyThing> keyFrames;
    keyFrames.addTime(0);
    keyFrames.addTime(1);

    KeyFrameValues<double, MyThing, MyThing_setWidth> widths;
    widths << 100 << 0;
    keyFrames.addValues(&widths);

    Animation animation;
    animation.setKeyFrames(&keyFrames);
    animation.setTarget(&thing);
    animation.tick(0.5); // will call MyThing_setWidth() with 50 and thing;
    \endcode

  */

template <typename Target>
class KeyFrames
{
public:
    ~KeyFrames() {
        for (auto i : m_values) delete i;
    }

    class Times : public std::vector<double> {
    public:
        Times &operator<<(double t) {
            assert(t >= 0); // sanity check range
            assert(t <= 1);
            assert(std::find(begin(), end(), t) == end()); // no diplicates
            assert(empty() || back() < t); // ensure sorting
            push_back(t);
            return *this;
        }
    };

    /*!
        Used to define the times for this keyframes object..

        Times must be between 0 and 1 and must be in sorted order.

        Once values have been added, it is an error to try add more key frame
        times.
     */
    Times &times() {
        assert(m_values.empty()); // no changes after adding values
        return m_times;
    }

    /*
        Add a set of values to the keyframes.

        The values must have the same number of entries as the number of times
        added to this keyframes and a set of key frame times must have been
        defined before the first set of values can be added.

     */
    template <typename Value, typename Callback>
    KeyFrameValues<Value, Target, Callback> &addValues() {
        assert(m_times.size() > 0);
        KeyFrameValues<Value, Target, Callback> *v = new KeyFrameValues<Value, Target, Callback>();
        m_values.push_back(v);
        return *v;
    }

    const Times &immutableTimes() const { return m_times; }
    const std::vector<KeyFrameValuesBase<Target> *> &values() const { return m_values; }

private:
    Times m_times;
    std::vector<KeyFrameValuesBase<Target> *> m_values;
};

template <typename Target,
          typename TimingFunction>
void Animation<Target, TimingFunction>::tick(double time)
{
    assert(isRunning());
    assert(m_keyFrames); // no animations without 'em.

    // cout << "tick: time=" << time << endl;
    bool stop = false;
    int curIter = (int) (time / m_duration);
    double timeInThisIteration = fmod(time, m_duration);
    if (m_iterations >= 0 && curIter >= m_iterations) {
        stop = true;
        timeInThisIteration = m_duration; // so we run until the very end..
        curIter = m_iterations - 1; // set to last iteration
        // cout << " - reached the end" << endl;
    }

    double scaledTime = timeInThisIteration / m_duration;

    // reverse direction if applicable
    if (m_direction == Reverse
        || (m_direction == Alternate && ((curIter % 2) == 1))
        || (m_direction == AlternateReverse && (curIter % 2 == 0))) {
        // cout << " - reversed..." << endl;
        scaledTime = 1.0 - scaledTime;
    }

    // Find the two indices to interpolate between
    int i0 = 0;
    int i1 = 1;
    const std::vector<double> &kft = m_keyFrames->immutableTimes();
    // we're before the thing starts...
    if (kft.front() > scaledTime) {
        i0 = i1 = 0;
    } else {
        while (i1 < kft.size() && kft.at(i1) < scaledTime)
            ++i1;
        // if (kft.at(i1) > scaledTime) // common case
        if (i1 >= kft.size())
            i0 = i1 = kft.size() - 1;
        else
            i0 = i1 - 1;
    }

    // cout << " - using: [ " << i0 << " -> " << i1 << " ] out of " << kft.size() << endl;

    assert(i0 >= 0);
    assert(i1 < kft.size());
    double t0 = kft.at(i0);
    double t1 = kft.at(i1);
    double t = (i0 < i1) ? (scaledTime - t0) / (t1 - t0) : t1;
    double et = m_timingFunction(t);

    // cout << " - time: [ " << t0 << " -> " << t1 << " ] t=" << t << "; eased=" << et << "; st=" << scaledTime << endl;

    for (auto v : m_keyFrames->values())
        v->interpolate(i0, i1, et, m_target);

    if (stop)
        setRunning(false);
}


template <typename Target,
          typename TimingFunction>
void Animation<Target, TimingFunction>::setKeyFrames(KeyFrames<Target> *kf)
{
    assert(!isRunning());
    for (auto i : kf->values())
        assert(kf->immutableTimes().size() == i->size());
    m_keyFrames = kf;
}

RENGINE_END_NAMESPACE