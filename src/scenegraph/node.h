#pragma once

#include <assert.h>
#include <vector>

namespace rengine {

class Node {
public:
    enum Type {
        BasicNodeType = 0,
        LayerNodeType,
        OpacityNodeType
    };

    /*!
     * Node constructor...
     */
    Node(Type type = BasicNodeType)
        : m_parent(0)
        , m_type(type)
    {
    }

    /*!
     * Node destructor.
     *
     * A Node will delete all its children when the destructor runs.
     */
    virtual ~Node() {
        if (m_parent)
            m_parent->remove(this);
        while (!m_children.empty())
            delete m_children.front();
    }

    /*!
     * Adds \a child at the end of this node's list of children.
     *
     * It is an error to add a child which already has a parent
     * or is already a child of this node.
     */
    void append(Node *child) {
        assert(std::find(m_children.begin(), m_children.end(), child) == m_children.end());
        m_children.push_back(child);
        child->setParent(this);
    }

    /*!
     * Adds \a child at the front of this node's list of children.
     *
     * It is an error to add a child which already has a parent
     * or is already a child of this node.
     */
    void prepend(Node *child) {
        assert(std::find(m_children.begin(), m_children.end(), child) == m_children.end());
        m_children.insert(m_children.begin(), child);
        child->setParent(this);
    }

    /*!
     * Removes \a child from this node's list of children.
     *
     * It is an error to remove a child which is not a child of
     * this node.
     */
    void remove(Node *child) {
        assert(std::find(m_children.begin(), m_children.end(), child) != m_children.end());
        m_children.erase(std::find(m_children.begin(), m_children.end(), child));
        child->setParent(0);
    }

    /*!
     * Returns the number of children in this node
     */
    int childCount() const { return m_children.size(); }

    /*!
     * Returns this node's list of children.
     */
    const std::vector<Node *> children() const { return m_children; }

    /*!
     * Returns this node's parent node.
     */
    Node *parent() const { return m_parent; }

    /*!
     * Returns this node's type.
     */
    Type type() const { return m_type; }

    /*!
     * Helper function to downcast from Node * to a specific subclass.
     */
    template <typename T>
    static T* from(Node *n) {
        assert(n);
        return (n->type() == T::StaticType) ? static_cast<T *>(n) : 0;
    }

    /*!
     * Helper function to downcast from const Node * to a specific subclass.
     */
    template <typename T>
    static const T* from(const Node *n) {
        assert(n);
        return (n->type() == T::StaticType) ? static_cast<const T *>(n) : 0;
    }

private:
    /*!
     * Sets this node's parent to \a p. This function is for internal use
     * only. Use append/prepend/remove from  public API.
     *
     * It is an error to set a parent on a node which already has a parent.
     */
    void setParent(Node *p) {
        assert(m_parent == 0 || p == 0);
        m_parent = p;
    }

    Node *m_parent;
    std::vector<Node *> m_children;

    Type m_type : 4;
};



class OpacityNode : public Node {
public:
    enum { StaticType = OpacityNodeType };
    OpacityNode()
        : Node(OpacityNodeType)
        , m_opacity(1.0f)
    {
    }

    float opacity() const { return m_opacity; }
    void setOpacity(float opacity) { m_opacity = opacity; }

private:
    float m_opacity;
};



class LayerNode : public Node {
public:
    enum { StaticType = LayerNodeType };
    LayerNode()
        : Node(LayerNodeType)
        , m_layer(0)
    {
    }

    Layer *layer() const { return m_layer; }
    void setLayer(Layer *layer) { m_layer = layer; }

    float x() const { return m_pos.x; }
    float y() const { return m_pos.y; }
    vec2 position() const { return m_pos; }
    void setPosition(float x, float y) { setPosition(vec2(x, y)); }
    void setPosition(const vec2 &pos) { m_pos = pos; }

    float width() const { return m_size.x; }
    float height() const { return m_size.y; }
    vec2 size() const { return m_size; }
    void setSize(float w, float h) { setSize(vec2(w, h)); }
    void setSize(const vec2 &size) { m_size = size; }

    void setGeometry(float x, float y, float w, float h) {
        setPosition(x, y);
        setSize(w, h);
    }

private:
    vec2 m_pos;
    vec2 m_size;
    Layer *m_layer;
};

} // end namespace rengine