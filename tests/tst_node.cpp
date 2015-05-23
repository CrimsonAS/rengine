
#include "test.h"

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
    check_true(tst_node_cast_helper<OpacityNode>());
    check_true(tst_node_cast_helper<LayerNode>());

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


void tst_layernode_geometry()
{
    LayerNode layerNode;

    layerNode.setSize(3, 4);
    // check_equal(layerNode.x(), 1);
    // check_equal(layerNode.y(), 2);
    check_equal(layerNode.width(), 3);
    check_equal(layerNode.height(), 4);
    // check_equal(layerNode.position(), vec2(1, 2));
    check_equal(layerNode.size(), vec2(3, 4));

    cout << __FUNCTION__ << ": ok" << endl;
}


int main(int, char **)
{
    tst_node_cast();
    tst_node_addRemoveParent();
    tst_layernode_geometry();

    return 0;
}
