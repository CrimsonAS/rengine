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

#pragma once

#include <assert.h>
#include <vector>
#include <algorithm>
#include <iostream>

RENGINE_BEGIN_NAMESPACE

class Node {
public:
    enum Type {
        BasicNodeType = 0,
        RectangleNodeType,
        LayerNodeType,
        TransformNodeType,
        OpacityNodeType,
        ColorFilterNodeType,
        BlurNodeType,
        ShadowNodeType,
    };

    /*!
     * Searches this node's list of children and returns true if \a child is a
     * child of this node.
     */
    bool hasChild(Node *child) {
        Node *n = m_child;
        while (n && n != child)
            n = n->m_sibling;
        return n;
    }

    /*!
     * Adds \a child at the end of this node's list of children.
     *
     * It is an error to add a child which already has a parent
     * or is already a child of this node.
     */
    void append(Node *child) {
        assert(!hasChild(child));
        assert(child->m_parent == 0);
        assert(child->m_sibling == 0);
        if (!m_lastChild) {
            assert(!m_child);
            m_child = child;
            m_lastChild = child;
        } else {
            assert(m_child);
            m_lastChild->m_sibling = child;
            m_lastChild = child;
        }
        child->setParent(this);
    }

    Node &operator<<(Node *child) { append(child); return *this; }

    /*!
     * Adds \a child at the front of this node's list of children.
     *
     * It is an error to add a child which already has a parent
     * or is already a child of this node.
     */
    void prepend(Node *child) {
        assert(!hasChild(child));
        assert(child->m_parent == 0);
        assert(child->m_sibling == 0);
        if (!m_child) {
            assert(!m_lastChild);
            m_child = child;
            m_lastChild = child;
        } else {
            assert(m_lastChild);
            child->m_sibling = m_child;
            m_child = child;
        }
        child->setParent(this);
    }

    /*!
     * Removes \a child from this node's list of children.
     *
     * It is an error to remove a child which is not a child of
     * this node.
     */
    void remove(Node *child) {
        assert(child);
        assert(hasChild(child));

        if (m_child == child) {
            m_child = m_child->m_sibling;
            if (child == m_lastChild)
                m_lastChild = 0;
        } else {
            Node *n = m_child;
            while (n->m_sibling != child) {
                n = n->m_sibling;
                assert(n);
            }
            n->m_sibling = child->m_sibling;
            if (child == m_lastChild) {
                m_lastChild = n;
                assert(n->m_sibling == 0);
            }
        }
        child->setParent(0);
    }

    // /*!
    //  * Injects this node into the tree above \a node. This ndoe becomes a
    //  * parent for \a node and will have the same order in the original parent's child list.
    //  */
    // void injectAbove(Node *node) {
    //     assert(node);
    //     assert(node->parent());
    //     assert(!parent());

    //     // Attach ourselves to the node's parent
    //     Node *p = node->parent();
    //     auto pos = std::find(p->m_children.begin(), p->m_children.end(), node);
    //     assert(pos != p->m_children.end());
    //     *pos = this;
    //     m_parent = p;

    //     // attach node to ourselves
    //     m_children.push_back(node);
    //     node->m_parent = this;
    // }

    // /*!
    //  * Removes this node from the tree and assigns all of its children to this
    //  * node's parent.
    //  */
    // void evict() {
    //     assert(parent());
    //     Node *p = parent();
    //     auto pos = std::find(p->m_children.begin(), m_children.end(), this);
    //     assert(pos != p->m_children.end());
    //     pos = p->m_children.erase(pos);
    //     for (auto i : m_children) {
    //         pos = p->m_children.insert(pos, i);
    //         i->m_parent = p;
    //     }
    //     m_children.clear();
    //     m_parent = 0;
    // }

    /*!
     * Returns the number of children in this node
     */
    int childCount() const {
        int c = 0;
        Node *n = m_child;
        while (n) {
            ++c;
            n = n->m_sibling;
        }
        return c;
    }

    /*!
     * Returns this node's parent node.
     */
    Node *parent() const { return m_parent; }

    Node *sibling() const { return m_sibling; }
    Node *child() const { return m_child; }

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
        return (n->type() == (Node::Type) T::StaticType) ? static_cast<T *>(n) : 0;
    }

    /*!
     * Helper function to downcast from const Node * to a specific subclass.
     */
    template <typename T>
    static const T* from(const Node *n) {
        assert(n);
        return (n->type() == T::StaticType) ? static_cast<const T *>(n) : 0;
    }

    void requestPreprocess() { m_preprocess = true; }
    void preprocess() {
        if (m_preprocess) {
            m_preprocess = false;
            onPreprocess();
        }
    }

    static void dump(Node *n, unsigned level = 0)
    {
        for (unsigned x=0; x<level; ++x) std::cout << " ";
        switch (n->type()) {
        case Node::BasicNodeType: std::cout << "Node"; break;
        case Node::OpacityNodeType: std::cout << "OpacityNode"; break;
        case Node::TransformNodeType: std::cout << "TransormNode"; break;
        case Node::RectangleNodeType: std::cout << "RectangleNode"; break;
        case Node::LayerNodeType: std::cout << "LayerNodeType"; break;
        default: std::cout << "Node(type=" << n->type() << ")"; break;
        }
        std::cout << "(" << n << ") parent=" << n->parent() << " childCount=" << n->childCount() << std::endl;
        Node *child = n->m_child;
        while (child) {
            dump(child, level + 1);
            child = child->m_sibling;
        }
    }

    RENGINE_ALLOCATION_POOL_DECLARATION(Node);

    void __mark_as_pool_allocated() { m_poolAllocated = true; }
    bool __is_pool_allocated() const { return m_poolAllocated; }

protected:
    virtual void onPreprocess() { }

    /*!
     * Node constructor...
     */
    Node(Type type = BasicNodeType)
        : m_parent(0)
        , m_child(0)
        , m_lastChild(0)
        , m_sibling(0)
        , m_type(type)
        , m_preprocess(false)
        , m_poolAllocated(false)
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
        while (m_child)
            m_child->destroy();
    }



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
    Node *m_child;
    Node *m_lastChild;
    Node *m_sibling;

    Type m_type : 4;
    unsigned m_preprocess : 1;
    unsigned m_poolAllocated : 1;
};

class OpacityNode : public Node {
public:
    enum { StaticType = OpacityNodeType };

    float opacity() const { return m_opacity; }
    void setOpacity(float opacity) { m_opacity = opacity; }

    RENGINE_ALLOCATION_POOL_DECLARATION(OpacityNode);

    static OpacityNode *create(float opacity) {
        auto node = create();
        node->setOpacity(opacity);
        return node;
    }

protected:
    OpacityNode()
        : Node(OpacityNodeType)
        , m_opacity(1.0f)
    {
    }

    float m_opacity;
};


class TransformNode : public Node
{
public:
    enum { StaticType = TransformNodeType };

    const mat4 &matrix() const { return m_matrix; }
    void setMatrix(const mat4 &m) { m_matrix = m; }

    float projectionDepth() const { return m_projectionDepth; }
    void setProjectionDepth(float d) { m_projectionDepth = d; }

    RENGINE_ALLOCATION_POOL_DECLARATION(TransformNode);

    static TransformNode *create(const mat4 &matrix, float projectionDepth = 0) {
        auto node = create();
        node->setMatrix(matrix);
        node->setProjectionDepth(projectionDepth);
        return node;
    }

protected:
    TransformNode()
        : Node(TransformNodeType)
        , m_projectionDepth(0)
    {
    }

    mat4 m_matrix;
    float m_projectionDepth;
};


class RectangleNode : public Node {
public:
    enum { StaticType = RectangleNodeType };

    const rect2d &geometry() const { return m_geometry; }
    void setGeometry(const rect2d &rect) { m_geometry = rect; }

    const vec4 &color() const { return m_color; }
    void setColor(const vec4 &color) {
        m_color = color;
        m_color.x = std::max(std::min(m_color.x, 1.0f), 0.0f);
        m_color.y = std::max(std::min(m_color.y, 1.0f), 0.0f);
        m_color.z = std::max(std::min(m_color.z, 1.0f), 0.0f);
        m_color.w = std::max(std::min(m_color.w, 1.0f), 0.0f);
    }

    RENGINE_ALLOCATION_POOL_DECLARATION(RectangleNode);

    static RectangleNode *create(const rect2d &geometry, const vec4 &color = vec4()) {
        auto node = create();
        node->setGeometry(geometry);
        node->setColor(color);
        return node;
    }

protected:
    RectangleNode(Type type = RectangleNodeType) : Node(type) { }

    rect2d m_geometry;
    vec4 m_color;
};


class LayerNode : public Node {
public:
    enum { StaticType = LayerNodeType };
    const Layer *layer() const { return m_layer; }
    void setLayer(const Layer *layer) { m_layer = layer; }

    const rect2d &geometry() const { return m_geometry; }
    void setGeometry(const rect2d &rect) { m_geometry = rect; }


    RENGINE_ALLOCATION_POOL_DECLARATION(LayerNode);

    static LayerNode *create(const rect2d &geometry, const Layer *layer) {
        auto node = create();
        node->setGeometry(geometry);
        node->setLayer(layer);
        return node;
    }

protected:
    LayerNode()
        : Node(LayerNodeType)
        , m_layer(0)
    {
    }

    const Layer *m_layer;
    rect2d m_geometry;
};

class ColorFilterNode : public Node {
public:
    enum { StaticType = ColorFilterNodeType };

    void setColorMatrix(const mat4 &matrix) { m_colorMatrix = matrix; }
    const mat4 &colorMatrix() const { return m_colorMatrix; }

    RENGINE_ALLOCATION_POOL_DECLARATION(ColorFilterNode);

    ColorFilterNode *create(const mat4 &matrix) {
        auto node = create();
        node->setColorMatrix(matrix);
        return node;
    }

protected:
    ColorFilterNode()
        : Node(ColorFilterNodeType)
    {
    }

    mat4 m_colorMatrix;
};

class BlurNode : public Node {
public:
    enum { StaticType = BlurNodeType };

    void setRadius(unsigned radius) { m_radius = radius; }
    unsigned radius() const { return m_radius; }

    RENGINE_ALLOCATION_POOL_DECLARATION(BlurNode);

    static BlurNode *create(unsigned radius) {
        auto node = create();
        node->setRadius(radius);
        return node;
    }

protected:
    BlurNode() : Node(BlurNodeType), m_radius(3) { }

    unsigned m_radius;
};

class ShadowNode : public Node {
public:
    enum { StaticType = ShadowNodeType };

    void setRadius(unsigned radius) { m_radius = radius; }
    unsigned radius() const { return m_radius; }

    void setOffset(const vec2 &offset) { m_offset = offset; }
    const vec2 &offset() const { return m_offset; }

    void setColor(const vec4 &color) { m_color = color; }
    const vec4 &color() const { return m_color; }

    RENGINE_ALLOCATION_POOL_DECLARATION(ShadowNode);

    static ShadowNode *create(unsigned radius, const vec2 &offset, const vec4 &color) {
        auto node = create();
        node->setRadius(radius);
        node->setOffset(offset);
        node->setColor(color);
        return node;
    }

protected:
    ShadowNode() : Node(ShadowNodeType), m_radius(3), m_offset(5, 5), m_color(0.5) { }

    unsigned m_radius;
    vec2 m_offset;
    vec4 m_color;
};



RENGINE_END_NAMESPACE
