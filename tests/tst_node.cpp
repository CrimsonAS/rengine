
#include <assert.h>
#include <iostream>

#include "node.h"

using namespace Rendgine::SceneGraph;
using namespace std;

template <typename T> bool tst_node_cast_helper()
{
    T org;
    T *casted = Node::from<T>(&org);
    return casted == &org;
    // const Node *cn = &org;
    // const T *constCasted = Node::from<const T>(cn);
    // return casted == &org && constCasted == &org;
}

void tst_node_cast()
{
    assert(tst_node_cast_helper<OpacityNode>());

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
    assert(n.childCount() == 2);
    assert(n1.childCount() == 2);
    assert(n2.childCount() == 1);

    // check parents
    assert(n.parent() == 0);
    assert(n1.parent() == &n);
    assert(n2.parent() == &n);
    assert(n11.parent() == &n1);
    assert(n12.parent() == &n1);
    assert(n21.parent() == &n2);

    // check children
    assert(n.children().at(0) == &n1);
    assert(n.children().at(1) == &n2);
    assert(n1.children().at(0) == &n11);
    assert(n1.children().at(1) == &n12);
    assert(n2.children().at(0) == &n21);

    /* move n21 to the front of n1's children
                n
               / \
             n1   n2
            / | \
         n21 n11 n12
     */
    n2.remove(&n21);
    n1.prepend(&n21);
    assert(n2.childCount() == 0);
    assert(n1.childCount() == 3);
    assert(n1.children().at(0) == &n21);
    assert(n1.children().at(1) == &n11);
    assert(n1.children().at(2) == &n12);
    assert(n21.parent() == &n1);

    cout << __FUNCTION__ << ": ok" << endl;
}


int main(int, char **)
{
    tst_node_cast();
    tst_node_addRemoveParent();

    return 0;
}
