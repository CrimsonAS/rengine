#include "rengine.h"
#include "test.h"

#include <vector>

using namespace rengine;
using namespace std;



class KeyFrameValuesBase
{
public:
    virtual void apply(int i0, int i1, double v) = 0;
    virtual int size() const = 0;
};



template <typename T>
class KeyFrameValues : public KeyFrameValuesBase
{
public:
    void apply(int i0, int i1, double t) {
        assert(i0 >= 0);
        assert(i0 < values.size() - 1);
        assert(i1 > 0);
        assert(i1 < values.size());
        assert(t >= 0);
        assert(t <= 1);
        const T &t0 = values.at(i0);
        const T &t1 = values.at(i1);
        apply(t0 + (t1 - t0) * t);
    }

    virtual void apply(const T &t) {
        cout << "applied value: " << t << endl;
    }

    void append(const T &v) {
        values.push_back(v);
    }

    int size() const { return values.size(); }

    vector<T> values;
};



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

    void addValues(KeyFrameValuesBase *v) {
        assert(m_times.size() > 0);
        assert(v->size() == m_times.size()); // these must line up...
        m_values.push_back(v);
    }

    const vector<double> &times() const { return m_times; }
    const vector<KeyFrameValuesBase *> &values() const { return m_values; }

private:
    vector<double> m_times;
    vector<KeyFrameValuesBase *> m_values;
};



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
      , m_iterations(1)
      , m_duration(1)
      , m_running(false)
      , m_direction(Normal)
    {
    }

    void tick(double time) {
        assert(isRunning());
        assert(m_keyFrames); // no animations without 'em.

        cout << "tick: time=" << time << endl;
        bool stop = false;
        int curIter = (int) (time / m_duration);
        if (m_iterations >= 0 && curIter >= m_iterations) {
            stop = true;
            time = m_duration; // so we run until the very end..
        }

        double timeInThisIteration = fmod(time, m_duration);
        double scaledTime = timeInThisIteration / m_duration;

        // ### TODO: this might not be padded all the way to the end..
        // 0% ...
        // 25% ...
        // 70% ...
        // and not 100%. Then we would have to pad it as well..
        int i0 = 0;
        int i1 = 1;
        const vector<double> &kft = m_keyFrames->times();
        // cout << " - times: ";
        // for (auto d : kft) cout << " " << d;
        // cout << endl;
        // cout << kft.at(0) << " " <<  scaledTime << " " << (kft.at(0) > scaledTime) << endl;

        // we're before the thing starts...
        if (kft.front() > scaledTime) {
            i0 = i1 = 0;
        } else {
            while (kft.at(i1) < scaledTime)
                ++i1;
            i0 = i1 - 1;
        }

        assert(i0 >= 0);
        assert(i1 < kft.size());
        double t0 = kft.at(i0);
        double t1 = kft.at(i1);
        double t = (scaledTime - t0) / (t1 - t0);

        cout << " - using: [ " << i0 << " -> " << i1 << " ] out of " << kft.size()
             << "; time: [ " << t0 << " -> " << t1 << " ] t=" << t << "; st=" << scaledTime << endl;

        // ### TODO: pass t through easing function here
        for (auto v : m_keyFrames->values())
            v->apply(i0, i1, t);

        if (stop)
            setRunning(false);
    }


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

     void setKeyFrames(KeyFrames *kf) {
         m_keyFrames = kf;
     }

private:
    KeyFrames *m_keyFrames;
    int m_iterations;
    double m_duration;

    unsigned m_running : 1;
    unsigned m_direction : 2;


};

void tst_keyframes_basic()
{
    Animation animation;
    animation.setDuration(10);
    // animation.easing = Animation::EaseIn;
    // animation.direction = Animation::Alternate;
    animation.setIterations(1);

    KeyFrames keyFrames;
    keyFrames.addTime(0);
    keyFrames.addTime(0.6);
    keyFrames.addTime(1);

    KeyFrameValues<vec4> *colors = new KeyFrameValues<vec4>();
    colors->append(vec4(1, 0, 0, 1));
    colors->append(vec4(0, 1, 0, 1));
    colors->append(vec4(0, 0, 1, 1));
    keyFrames.addValues(colors);

    KeyFrameValues<double> *opacities = new KeyFrameValues<double>();
    opacities->append(1);
    opacities->append(1);
    opacities->append(0);
    keyFrames.addValues(opacities);

    animation.setKeyFrames(&keyFrames);

    animation.setRunning(true);
    for (int i=0; i<=animation.duration() * animation.iterations(); ++i) {
        animation.tick(i);
    }
    assert(!animation.isRunning());

    cout << __FUNCTION__ << ": ok" << endl;
}

int main(int argc, char **argv)
{

    tst_keyframes_basic();

}