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

#define RENGINE_NODE_DEFINE_FROM_FUNCTION(Class, Type) \
    static Class *from(Node *node) {                   \
        return (node->type() == Type)                  \
               ? static_cast<Class *>(node)            \
               : 0;                                    \
    }                                                  \
    static const Class *from(const Node *node) {       \
        return (node->type() == Type)                  \
               ? static_cast<const Class *>(node)      \
               : 0;                                    \
    }                                                  \

// ### Is this the right place for this macro?
#define RENGINE_IMPLEMENT_PROPERTY(type, name, setter, signal) \
    protected:                                \
        type m_##name;                        \
    public:                                   \
        static Signal<> signal;               \
        type (name)() const {                 \
            return m_##name;                  \
        }                                     \
        void setter(const type &name) {       \
            if (m_##name == name)             \
                return;                       \
            m_##name = name;                  \
            signal.emit(this);                \
        }

class Node : public SignalEmitter
{
public:
    enum Type {
        BasicNodeType         = 0,
        TransformNodeType     = 1,
        OpacityNodeType       = 2,
        ColorFilterNodeType   = 3,
        BlurNodeType          = 4,
        ShadowNodeType        = 5,

        RectangleNodeBaseType = (1 << 6),
        RectangleNodeType     = 0 | RectangleNodeBaseType,
        TextureNodeType       = 1 | RectangleNodeBaseType,
        RenderNodeType        = 2 | RectangleNodeBaseType,
    };

    /*!
     * Searches this node's list of children and returns true if \a child is a
     * child of this node.
     */
    bool hasChild(Node *child) {
        Node *n = m_child;
        while (n && n != child)
            n = n->sibling();
        return n;
    }

    /*!
     * Adds \a child at the end of this node's list of children.
     *
     * It is an error to add a child which already has a parent
     * or is already a child of this node.
     */
    void append(Node *child) {
        assert(child);
        assert(!hasChild(child));
        assert(child->m_parent == 0);
        assert(child->m_next == 0);
        assert(child->m_prev == 0);

        if (!m_child) {
            child->m_next = child;
            child->m_prev = child;
            m_child = child;
        } else {
            m_child->m_prev->m_next = child;
            child->m_prev = m_child->m_prev;
            m_child->m_prev = child;
            child->m_next = m_child;
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
        // Since children are a cyclic linked list, we can do prepend
        // in terms of an append, followed by setting the added child
        // as the front of the list.
        append(child);
        m_child = child;
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

        // only child..
        if (child->m_next == child) {
            m_child = 0;
        } else {
            if (m_child == child)
                m_child = child->m_next;
            child->m_next->m_prev = child->m_prev;
            child->m_prev->m_next = child->m_next;
        }
        child->m_next = 0;
        child->m_prev = 0;
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
     * Returns the number of children in this node.
     *
     * This calculation is O(n).
     */
    int childCount() const {
        int c = 0;
        Node *n = m_child;
        while (n) {
            ++c;
            n = n->sibling();
        }
        return c;
    }

    /*!
     * Returns this node's parent node.
     */
    Node *parent() const { return m_parent; }

    Node *sibling() const {
        assert(m_parent);
        return m_next == m_parent->m_child ? 0 : m_next;
    }
    Node *previousSibling() const {
        assert(m_parent);
        return this == m_parent->m_child ? 0 : m_prev;
    }

    Node *child() const { return m_child; }
    Node *lastChild() const {
        if (!m_child)
            return 0;
        else if (m_child->m_prev == m_child) // an only child
            return m_child;
        return m_child->m_prev;
    }

    /*!
     * Returns this node's type.
     */
    Type type() const { return m_type; }

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
        case Node::TextureNodeType: std::cout << "TextureNodeType"; break;
        default: std::cout << "Node(type=" << n->type() << ")"; break;
        }
        std::cout << "(" << n << ") parent=" << n->parent()
                  << " childCount=" << n->childCount()
                  << " next=" << n->m_next
                  << " prev=" << n->m_prev
                  << std::endl;
        Node *child = n->m_child;
        while (child) {
            dump(child, level + 1);
            child = child->sibling();
        }
    }

    RENGINE_ALLOCATION_POOL_DECLARATION_IN_BASECLASS(Node, rengine_Node);

    void __mark_as_pool_allocated() { m_poolAllocated = true; }
    bool __is_pool_allocated() const { return m_poolAllocated; }

    /*!
     * State variable used by the event dispatch.
     */
    bool isPointerTarget() const { return m_pointerTarget; }
    void setPointerTarget(bool target) { m_pointerTarget = target;}

protected:
    virtual void onPreprocess() { }

    /*!
     * Node constructor...
     */
    Node(Type type = BasicNodeType)
        : m_parent(0)
        , m_child(0)
        , m_next(0)
        , m_prev(0)
        , m_type(type)
        , m_preprocess(false)
        , m_poolAllocated(false)
        , m_pointerTarget(false)
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
    Node *m_next;
    Node *m_prev;

    Type m_type : 8;
    unsigned m_preprocess : 1;
    unsigned m_poolAllocated : 1;
    unsigned m_pointerTarget : 1;
    unsigned m_pointerDisabled : 1;
    unsigned m_reserved : 20; // 32 - 12
};

class OpacityNode : public Node {
public:
    float opacity() const { return m_opacity; }
    void setOpacity(float opacity) { m_opacity = opacity; }

    RENGINE_ALLOCATION_POOL_DECLARATION(OpacityNode, rengine_OpacityNode);

    static OpacityNode *create(float opacity) {
        auto node = create();
        node->setOpacity(opacity);
        return node;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(OpacityNode, OpacityNodeType);

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
    mat4 matrix() const { return m_matrix; }
    void setMatrix(mat4 m) { m_matrix = m; }

    float projectionDepth() const { return m_projectionDepth; }
    void setProjectionDepth(float d) { m_projectionDepth = d; }

    RENGINE_ALLOCATION_POOL_DECLARATION(TransformNode, rengine_TransformNode);

    static TransformNode *create(mat4 matrix, float projectionDepth = 0) {
        auto node = create();
        node->setMatrix(matrix);
        node->setProjectionDepth(projectionDepth);
        return node;
    }

    static mat4 matrixFor(Node *descendant, Node *root = 0) {
        Node *n = descendant;
        mat4 m;
        while (true) {
            if (TransformNode *tn = TransformNode::from(n)) {
                m = tn->matrix() * m;
            }
            if (n == root)
                break;
            n = n->parent();
            assert(root == nullptr || n != nullptr);
        }
        return m;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(TransformNode, TransformNodeType);

protected:
    TransformNode()
        : Node(TransformNodeType)
        , m_projectionDepth(0)
    {
    }

    mat4 m_matrix;
    float m_projectionDepth;
};

class SimplifiedTransformNode : public TransformNode
{
public:

    RENGINE_ALLOCATION_POOL_DECLARATION(SimplifiedTransformNode, rengine_SimplifiedTransformNode);

    static Signal<> onDxChanged;
    static Signal<> onDyChanged;
    static Signal<> onRotationAroundXChanged;
    static Signal<> onRotationAroundYChanged;
    static Signal<> onRotationAroundZChanged;
    static Signal<> onRotationChanged;
    static Signal<> onScaleChanged;

    float dx() const { return m_dx; }
    void setDx(float dx) {
        if (m_dx == dx)
            return;
        m_dx = dx;
        onDxChanged.emit(this);
        requestPreprocess();
    }

    float dy() const { return m_dy; }
    void setDy(float dy) {
        if (m_dy == dy)
            return;
        m_dy = dy;
        onDyChanged.emit(this);
        requestPreprocess();
    }

    RENGINE_IMPLEMENT_PROPERTY(float, dz, setDz, onDzChanged);

    float rotationAroundX() const { return m_rotx; }
    void setRotationAroundX(float rx) {
        if (m_rotx == rx)
            return;
        m_rotx = rx;
        onRotationAroundXChanged.emit(this);
        requestPreprocess();
    }

    float rotationAroundY() const { return m_roty; }
    void setRotationAroundY(float ry) {
        if (m_roty == ry)
            return;
        m_roty = ry;
        onRotationAroundYChanged.emit(this);
        requestPreprocess();
    }

    float rotationAroundZ() const { return m_rotz; }
    void setRotationAroundZ(float rz) {
        if (m_rotz == rz)
            return;
        m_rotz = rz;
        onRotationAroundZChanged.emit(this);
        requestPreprocess();
    }

    float rotation() const { return rotationAroundZ(); }
    void setRotation(float rz) { setRotationAroundZ(rz); }

    float scale() const { return m_scale; }
    void setScale(float s) {
        if (m_scale == s)
            return;
        m_scale = s;
        onScaleChanged.emit(this);
        requestPreprocess();
    }

    void onPreprocess() override {
        mat4 m;
        if (m_dz)
            m = m * mat4::translate(0, 0, m_dz);
        if (m_dx != 0 || m_dy != 0)
            m = m * mat4::translate2D(m_dx, m_dy);
        if (m_scale != 0)
            m = mat4::scale2D(m_scale, m_scale);
        if (m_rotx != 0)
            m = m * mat4::rotateAroundX(m_rotx);
        if (m_roty != 0)
            m = m * mat4::rotateAroundY(m_roty);
        if (m_rotz != 0)
            m = m * mat4::rotateAroundZ(m_rotz);
        setMatrix(m);
    }

protected:
    float m_dx;
    float m_dy;
    float m_rotx;
    float m_roty;
    float m_rotz;
    float m_scale;
};


class RectangleNodeBase : public Node {
public:

    static Signal<> onXChanged;
    static Signal<> onYChanged;
    static Signal<> onWidthChanged;
    static Signal<> onHeightChanged;

    float x() const { return m_geometry.x(); }
    void setX(float x) {
        if (x == m_geometry.x())
            return;
        m_geometry.setX(x);
        onXChanged.emit(this);
    }

    float y() const { return m_geometry.y(); }
    void setY(float y) {
        if (y == m_geometry.y())
            return;
        m_geometry.setY(y);
        onYChanged.emit(this);
    }

    vec2 position() const { return m_geometry.position(); }
    void setPosition(vec2 position) {
        setX(position.x);
        setY(position.y);
    }
    void setPosition(float x, float y) {
        setX(x);
        setY(y);
    }

    float width() const { return m_geometry.width(); }
    void setWidth(float w) {
        if (w == m_geometry.width())
            return;
        m_geometry.setWidth(w);
        onWidthChanged.emit(this);
    }

    float height() const { return m_geometry.height(); }
    void setHeight(float h) {
        if (h == m_geometry.height())
            return;
        m_geometry.setHeight(h);
        onHeightChanged.emit(this);
    }

    rect2d geometry() const { return m_geometry; }
    void setGeometry(rect2d rect) {
        bool updateX = rect.x() != m_geometry.x();
        bool updateY = rect.y() != m_geometry.y();
        bool updateW = rect.width() != m_geometry.width();
        bool updateH = rect.height() != m_geometry.height();
        if (!updateX && !updateY && !updateW && !updateH)
            return;
        m_geometry = rect;
        if (updateX) onXChanged.emit(this);
        if (updateY) onYChanged.emit(this);
        if (updateW) onWidthChanged.emit(this);
        if (updateH) onHeightChanged.emit(this);
    }

    // Uses & BaseType, so can't use the macro
    static RectangleNodeBase *from(Node *node) {
        return (node->type() & RectangleNodeBaseType)
               ? static_cast<RectangleNodeBase *>(node)
               : 0;
    }
    static const RectangleNodeBase *from(const Node *node) {
        return (node->type() & RectangleNodeBaseType)
               ? static_cast<const RectangleNodeBase *>(node)
               : 0;
    }

protected:
    RectangleNodeBase(Type type)
        : Node(type)
    {
        assert(type & RectangleNodeBaseType);
    }

    rect2d m_geometry;
};


class RectangleNode : public RectangleNodeBase {
public:

    static Signal<> onColorChanged;

    vec4 color() const { return m_color; }
    void setColor(vec4 color) {
        vec4 c = color;
        c.x = std::max(std::min(c.x, 1.0f), 0.0f);
        c.y = std::max(std::min(c.y, 1.0f), 0.0f);
        c.z = std::max(std::min(c.z, 1.0f), 0.0f);
        c.w = std::max(std::min(c.w, 1.0f), 0.0f);
        if (c == m_color)
            return;
        m_color = color;
        onColorChanged.emit(this);
    }

    RENGINE_ALLOCATION_POOL_DECLARATION(RectangleNode, rengine_RectangleNode);

    static RectangleNode *create(rect2d geometry, vec4 color = vec4()) {
        auto node = create();
        node->setGeometry(geometry);
        node->setColor(color);
        return node;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(RectangleNode, RectangleNodeType);

protected:
    RectangleNode(Type type = RectangleNodeType) : RectangleNodeBase(type) { }

    vec4 m_color;
};


class TextureNode : public RectangleNodeBase {
public:
    const Texture *texture() const { return m_texture; }
    void setTexture(const Texture *texture) { m_texture = texture; }

    RENGINE_ALLOCATION_POOL_DECLARATION(TextureNode, rengine_TextureNode);

    static TextureNode *create(rect2d geometry, const Texture *texture) {
        auto node = create();
        node->setGeometry(geometry);
        node->setTexture(texture);
        return node;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(TextureNode, TextureNodeType);

protected:
    TextureNode()
        : RectangleNodeBase(TextureNodeType)
    {
    }

    const Texture *m_texture = nullptr;
};

class ColorFilterNode : public Node {
public:
    void setColorMatrix(mat4 matrix) { m_colorMatrix = matrix; }
    mat4 colorMatrix() const { return m_colorMatrix; }

    RENGINE_ALLOCATION_POOL_DECLARATION(ColorFilterNode, rengine_ColorFilterNode);

    ColorFilterNode *create(mat4 matrix) {
        auto node = create();
        node->setColorMatrix(matrix);
        return node;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(ColorFilterNode, ColorFilterNodeType);

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

    RENGINE_ALLOCATION_POOL_DECLARATION(BlurNode, rengine_BlurNode);

    static BlurNode *create(unsigned radius) {
        auto node = create();
        node->setRadius(radius);
        return node;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(BlurNode, BlurNodeType);

protected:
    BlurNode() : Node(BlurNodeType), m_radius(3) { }

    unsigned m_radius;
};

class ShadowNode : public Node {
public:
    enum { StaticType = ShadowNodeType };

    void setRadius(unsigned radius) { m_radius = radius; }
    unsigned radius() const { return m_radius; }

    void setOffset(vec2 offset) { m_offset = offset; }
    vec2 offset() const { return m_offset; }

    void setColor(vec4 color) { m_color = color; }
    vec4 color() const { return m_color; }

    RENGINE_ALLOCATION_POOL_DECLARATION(ShadowNode, rengine_ShadowNode);

    static ShadowNode *create(unsigned radius, vec2 offset, vec4 color) {
        auto node = create();
        node->setRadius(radius);
        node->setOffset(offset);
        node->setColor(color);
        return node;
    }

    RENGINE_NODE_DEFINE_FROM_FUNCTION(ShadowNode, ShadowNodeType);

protected:
    ShadowNode() : Node(ShadowNodeType), m_radius(3), m_offset(5, 5), m_color(0.5) { }

    unsigned m_radius;
    vec2 m_offset;
    vec4 m_color;
};


class RenderNode : public RectangleNodeBase {
public:
    RENGINE_NODE_DEFINE_FROM_FUNCTION(RenderNode, RenderNodeType);

    /*
        The OpenGL code should take the following things into account when it
        is called:
         - GL_BLEND is enabled
         - GL_DEPTH_TEST is disabled
         - GL_STENCIL_TEST is disabled
         - glDepthMask(false) is used
         - glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA) is used
         - No buffers are bound and all attributes are disabled
         - No program is bound

        Upon leaving this function, the code should leave the OpenGL context
        in a similar state, especially with regards to array and index buffers
        and attribute registers.
     */
    virtual void render() = 0;


    // Does not allow pool allocation as it is pure virtual..

protected:
    RenderNode(Type type = RenderNodeType) : RectangleNodeBase(type) { }
};


#define RENGINE_NODE_DEFINE_ALLOCATION_POOLS                                                                  \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::Node, rengine_Node);                                          \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::TransformNode, rengine_TransformNode);                        \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::SimplifiedTransformNode, rengine_SimplifiedTransformNode);    \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::OpacityNode, rengine_OpacityNode);                            \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::TextureNode, rengine_TextureNode);                            \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::RectangleNode, rengine_RectangleNode);                        \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::ColorFilterNode, rengine_ColorFilterNode);                    \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::BlurNode, rengine_BlurNode);                                  \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::ShadowNode, rengine_ShadowNode);

#define RENGINE_NODE_DEFINE_SIGNALS                                                 \
                                                                                    \
    rengine::Signal<> rengine::RectangleNodeBase::onXChanged;                       \
    rengine::Signal<> rengine::RectangleNodeBase::onYChanged;                       \
    rengine::Signal<> rengine::RectangleNodeBase::onWidthChanged;                   \
    rengine::Signal<> rengine::RectangleNodeBase::onHeightChanged;                  \
                                                                                    \
    rengine::Signal<> rengine::RectangleNode::onColorChanged;                       \
                                                                                    \
    rengine::Signal<> rengine::SimplifiedTransformNode::onDxChanged;                \
    rengine::Signal<> rengine::SimplifiedTransformNode::onDyChanged;                \
    rengine::Signal<> rengine::SimplifiedTransformNode::onDzChanged;                \
    rengine::Signal<> rengine::SimplifiedTransformNode::onRotationAroundXChanged;   \
    rengine::Signal<> rengine::SimplifiedTransformNode::onRotationAroundYChanged;   \
    rengine::Signal<> rengine::SimplifiedTransformNode::onRotationAroundZChanged;   \
    rengine::Signal<> rengine::SimplifiedTransformNode::onRotationChanged;          \
    rengine::Signal<> rengine::SimplifiedTransformNode::onScaleChanged;             \

RENGINE_END_NAMESPACE
