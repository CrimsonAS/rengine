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


#include "test.h"

template <typename T> bool tst_node_cast_helper()
{
    Node *org = T::create();
    T *casted = T::from(org);
    return casted == org;
}

void tst_node_cast()
{
    check_true(tst_node_cast_helper<OpacityNode>());
    check_true(tst_node_cast_helper<TextureNode>());
    check_true(tst_node_cast_helper<TransformNode>());
    check_true(tst_node_cast_helper<RectangleNode>());
    check_true(tst_node_cast_helper<ColorFilterNode>());
    check_true(tst_node_cast_helper<BlurNode>());
    check_true(tst_node_cast_helper<ShadowNode>());

    cout << __FUNCTION__ << ": ok" << endl;
}

static void dumpToList_prefix(Node *n, std::vector<Node *> &list)
{
    list.push_back(n);
    Node *c = n->child();
    while (c) {
        dumpToList_prefix(c, list);
        c = c->sibling();
    }
}

void tst_node_addRemoveParent()
{
    Node *n = Node::create();
    Node *n1 = Node::create();
    Node *n2 = Node::create();
    Node *n11 = Node::create();
    Node *n12 = Node::create();
    Node *n21 = Node::create();

    // cout << "n ....: " << n << endl;
    // cout << "n1 ...: " << n1 << endl;
    // cout << "n2 ...: " << n2 << endl;
    // cout << "n11 ..: " << n11 << endl;
    // cout << "n12 ..: " << n12 << endl;
    // cout << "n21 ..: " << n21 << endl;
    /* build the tree
            n
           / \
         n1   n2
        /  \    \
      n11  n12   n21
    */

    n->append(n1);
    n->append(n2);
    n1->append(n11);
    n1->append(n12);
    n2->append(n21);

    // Check child counts
    check_equal(n->childCount(), 2);
    check_equal(n1->childCount(), 2);
    check_equal(n2->childCount(), 1);

    // check parents
    check_equal(n->parent(), 0);
    check_equal(n1->parent(), n);
    check_equal(n2->parent(), n);
    check_equal(n11->parent(), n1);
    check_equal(n12->parent(), n1);
    check_equal(n21->parent(), n2);

    // check the tree
    std::vector<Node *> dump;
    dumpToList_prefix(n, dump);
    check_equal(dump.at(0), n);
    check_equal(dump.at(1), n1);
    check_equal(dump.at(2), n11);
    check_equal(dump.at(3), n12);
    check_equal(dump.at(4), n2);
    check_equal(dump.at(5), n21);
    dump.clear();


    /* move n21 to the front of n1's children
                n
               / \
             n1   n2
            / | \
         n21 n11 n12
     */
    n2->remove(n21);
    n1->prepend(n21);

    check_equal(n2->childCount(), 0);
    check_equal(n1->childCount(), 3);
    check_equal(n21->parent(), n1);
    dumpToList_prefix(n, dump);
    check_equal(dump.at(0), n);
    check_equal(dump.at(1), n1);
    check_equal(dump.at(2), n21);
    check_equal(dump.at(3), n11);
    check_equal(dump.at(4), n12);
    check_equal(dump.at(5), n2);

    n->destroy();

    cout << __FUNCTION__ << ": ok" << endl;
}



void tst_node_allocator()
{
    RENGINE_ALLOCATION_POOL(Node, rengine_Node, 16);

    // Create the node
    Node *n = Node::create();
    check_true(n != 0);
    check_true(n->__is_pool_allocated())

    // Make sure destructors are called..
    bool written = false;
    class OtherNode : public Node {
    public:
        bool *m_write;
        OtherNode(bool *write) : m_write(write) { }
        ~OtherNode() { *m_write = true; }
    };
    n->append(new OtherNode(&written));
    n->destroy();
    check_true(written);


    cout << __FUNCTION__ << ": ok" << endl;
}

// static  void tst_node_injectEvict()
// {
//     Node root;
//     Node n1;
//     Node n2;
//     Node n11;
//     Node n12;
//     Node n21;
//     Node n22;
//     Node injected;

//     root << &n1 << &n2;
//     n1 << &n11 << &n12;
//     n2 << &n21 << &n22;

//     injected.injectAbove(&n1);
//     check_equal(2, root.childCount());
//     check_equal(1, injected.childCount());
//     check_equal(root.children().at(0), &injected);
//     check_equal(&root, injected.parent());
//     check_equal(n1.parent(), &injected);
//     check_equal(n11.parent(), &n1);
//     check_equal(n12.parent(), &n1);
//     check_equal(n21.parent(), &n2);
//     check_equal(n22.parent(), &n2);

//     injected.evict();
//     check_equal(injected.parent(), 0);
//     check_equal(injected.childCount(), 0);
//     check_equal(2, root.childCount());
//     check_equal(root.children().at(0), &n1);
//     check_equal(n1.parent(), &root);
//     check_equal(n11.parent(), &n1);
//     check_equal(n12.parent(), &n1);
//     check_equal(n21.parent(), &n2);
//     check_equal(n22.parent(), &n2);

//     cout << __FUNCTION__ << ": ok" << endl;
// }

void tst_noderef()
{
    // Node *node = Node::create();
    // NodeRef<Node> n(node);
    // check_true(!n.expired());
    // check_equal(n.get(), node);
    // check_equal(n->type(), Node::BasicNodeType);
    // check_equal(n->childCount(), 0);
    // node->destroy();
    // check_true(n.expired());
    // check_true(n.get() == nullptr);

    RectangleNode *rectNode = RectangleNode::create(rect2d(1, 2, 3, 4), vec4(0.1, 0.2, 0.3, 0.4));
    NodeRef<RectangleNode> r1, r2;
    r1 = NodeRef<RectangleNode>(rectNode);
    r2 = r1;
    check_equal(r1.get(), rectNode);
    check_equal(r2.get(), rectNode);
    check_true(!r1.expired());
    check_true(!r2.expired());
    check_equal(r1->geometry(), rect2d(1, 2, 3, 4));
    check_equal(r2->color(), vec4(0.1, 0.2, 0.3, 0.4));
    rectNode->destroy();
    check_true(r1.get() == nullptr);
    check_true(r2.get() == nullptr);
    check_true(r1.expired());
    check_true(r2.expired());

    cout << __FUNCTION__ << ": ok!" << endl;
}

int main(int, char **)
{
    // tst_node_cast();
    // tst_node_addRemoveParent();
    // tst_rectanglenode_geometry();
    // tst_node_injectEvict();

    // tst_node_allocator();

    tst_noderef();

    return 0;
}
