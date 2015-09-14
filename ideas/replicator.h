#pragma once

RENGINE_BEGIN_NAMESPACE


// replicator working as a more dynamic allocator class
template <typename CreateType>
class Replicator : public SignalEmitter
{
public:
    static Signal<> onCountChanged;

    int count() const { return m_count; }
    void setCount(int count) {
        if (m_count == count)
            return;
        m_count = count;
        onCountChanged.emit(&this);

        // Delete the old ones..
        for (int i=0; i<m_instances.size(); ++i) {
            onInstanceDestroyed(m_instances[i], i, m_instances.size());
            destroy(m_instances[i]);
        }

        // Create the new ones..
        for (int i=0; i<m_count; ++i) {
            T *t = create();
            onInstanceCreated(t, index, m_count);
        }
    }

    void onInstanceCreated(T *instance, int index, int count) {
        // We could also pass a reference to the class that owns the replicator.
        // That would be convenient way to access for instance a datamodel.
        float c = index / (float) count; instance->setColor(vec4(c, 0.5, 1.0 - c, 0.8));
    }

    void onInstanceDestroyed(T *instance, int index, int count) {
        // nada in this case...
    }

    T *create() {
        return T::create();
    }

    void destroy(T *t) {
        t->destroy();
    }

protected:
    int m_count;
    std::vector<T *> m_instances;
};

// replicator working as direct inline thing..
inline void setupObjects()
{
    const int count = 20;
    for (int index=0; index<count; ++index) {
        Billboard *instance = Billboard::create();
        float c = index / (float) count; instance->setColor(vec4(c, 0.5, 1.0 - c, 0.8));
    }
}

RENGINE_END_NAMESPACE