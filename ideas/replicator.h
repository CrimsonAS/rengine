#pragma once

RENGINE_BEGIN_NAMESPACE

// replicator working as a more dynamic allocator class
template <typename ReplicationHelper>
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
            m_helper.onInstanceDestroyed(m_instances[i], i, m_instances.size());
            m_helper.destroy(m_instances[i]);
        }

        // Create the new ones..
        for (int i=0; i<m_count; ++i) {
            T *t = m_helper.create();
            m_helper.onInstanceCreated(t, index, m_count);
        }
    }

protected:
    int m_count;
    std::vector<T *> m_instances;

    ReplicationHelper<> m_helper;
};

RENGINE_END_NAMESPACE