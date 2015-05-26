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

#pragma once

#include <vector>

RENGINE_BEGIN_NAMESPACE

class LinearTimingFunction
{
public:
    double operator()(double t) { return t; }
};

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

    Target *target() const { return m_target; }
    void setTarget(Target *t) { m_target = t; }

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
     void setDirection(Direction d) { m_direction = d; }

     bool isRunning() const { return m_running; }
     void setRunning(bool r) { m_running = r; }

     KeyFrames<Target> *keyFrames() const { return m_keyFrames; }
     void setKeyFrames(KeyFrames<Target> *kf) {
         m_keyFrames = kf;
     }

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

template <typename Target>
class KeyFrameValuesBase
{
public:
    virtual void interpolate(int i0, int i1, double v, Target *target) = 0;
    virtual int size() const = 0;
};

template <typename Value, typename Target>
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
        const Value &v0 = values.at(i0);
        const Value &v1 = values.at(i1);
        apply(v0 + (v1 - v0) * t, target);
    }

    virtual void apply(const Value &value, Target *target) = 0;

    void append(const Value &v) { values.push_back(v); }

    KeyFrameValues<Value, Target> &operator<<(const Value &v) { append(v); return *this; }

    int size() const { return values.size(); }

private:
    std::vector<Value> values;
};

template <typename Target>
class KeyFrames
{
public:
    void addTime(double t) {
        assert(t >= 0); // sanity check range
        assert(t <= 1);
        assert(std::find(m_times.begin(), m_times.end(), t) == m_times.end()); // no diplicates
        assert(m_times.empty() || m_times.back() < t); // ensure sorting
        assert(m_values.empty()); // no changes after adding values
        m_times.push_back(t);
    }

    void addValues(KeyFrameValuesBase<Target> *v) {
        assert(m_times.size() > 0);
        assert(v->size() == m_times.size()); // these must line up...
        m_values.push_back(v);
    }

    const std::vector<double> &times() const { return m_times; }
    const std::vector<KeyFrameValuesBase<Target> *> &values() const { return m_values; }

private:
    std::vector<double> m_times;
    std::vector<KeyFrameValuesBase<Target> *> m_values;
};

template <typename TimingFunction,
          typename Target>
void Animation<TimingFunction, Target>::tick(double time)
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
    const std::vector<double> &kft = m_keyFrames->times();
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


RENGINE_END_NAMESPACE