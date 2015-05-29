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


#include "test.h"

template <typename T> bool tst_node_cast_helper()
{
    T org;
    T *casted = Node::from<T>(&org);
    return casted == &org;
}

void tst_node_cast()
{
    check_true(tst_node_cast_helper<OpacityNode>());
    check_true(tst_node_cast_helper<LayerNode>());
    check_true(tst_node_cast_helper<TransformNode>());
    check_true(tst_node_cast_helper<RectangleNode>());

    cout << __FUNCTION__ << ": ok" << endl;
}



void tst_node_addRemoveParent()
{
    Node n, n1, n2, n11, n12, n21;

    /* build the tree
            n
           / \
         n1   n2
        /  \    \
      n11  n12   n21
    */

    n.append(&n1);
    n.append(&n2);
    n1.append(&n11);
    n1.append(&n12);
    n2.append(&n21);

    // Check child counts
    check_equal(n.childCount(), 2);
    check_equal(n1.childCount(), 2);
    check_equal(n2.childCount(), 1);

    // check parents
    check_equal(n.parent(), 0);
    check_equal(n1.parent(), &n);
    check_equal(n2.parent(), &n);
    check_equal(n11.parent(), &n1);
    check_equal(n12.parent(), &n1);
    check_equal(n21.parent(), &n2);

    // check children
    check_equal(n.children().at(0), &n1);
    check_equal(n.children().at(1), &n2);
    check_equal(n1.children().at(0), &n11);
    check_equal(n1.children().at(1), &n12);
    check_equal(n2.children().at(0), &n21);

    /* move n21 to the front of n1's children
                n
               / \
             n1   n2
            / | \
         n21 n11 n12
     */
    n2.remove(&n21);
    n1.prepend(&n21);
    check_equal(n2.childCount(), 0);
    check_equal(n1.childCount(), 3);
    check_equal(n1.children().at(0), &n21);
    check_equal(n1.children().at(1), &n11);
    check_equal(n1.children().at(2), &n12);
    check_equal(n21.parent(), &n1);

    cout << __FUNCTION__ << ": ok" << endl;
}


void tst_rectanglenode_geometry()
{
    LayerNode layerNode;

    layerNode.setPosition(1, 2);
    layerNode.setSize(3, 4);
    check_equal(layerNode.x(), 1);
    check_equal(layerNode.y(), 2);
    check_equal(layerNode.width(), 3);
    check_equal(layerNode.height(), 4);
    check_equal(layerNode.position(), vec2(1, 2));
    check_equal(layerNode.size(), vec2(3, 4));

    cout << __FUNCTION__ << ": ok" << endl;
}


int main(int, char **)
{
    tst_node_cast();
    tst_node_addRemoveParent();
    tst_rectanglenode_geometry();

    return 0;
}
