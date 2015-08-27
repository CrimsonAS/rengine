#include "test.h"

void tst_property()
{
    // Create my property
    Property<int> x;

    // Check that it is default constructed to T() aka 0
    check_equal(x, 0);

    // Check that notification triggers and can update a local var
    int copyOfX = 0;
    x.connect([&]() { copyOfX = x; });
    x = 1;
    check_equal(x, 1);
    check_equal(copyOfX, 1);

    // Check that we can set up a bidning to another property
    bool wasCalled = false;
    Property<int> *other = new Property<int>();
    int id = x.connect([&] {
        other->set(x + 100);
        wasCalled = true;
    });
    x = 2;
    check_equal(x, 2);
    check_equal(*other, 102);

    // Check that we can unregister and that the dependent value is not called
    wasCalled = false;
    x.disconnect(id);
    delete other;
    x = 3;
    check_equal(x, 3);
    check_equal(wasCalled, false);

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}


void tst_bounded_property()
{
    BoundedProperty<float> float_under { 0.0f, 1.0f, -2.0f };
    check_equal(float_under, 0.0f);

    BoundedProperty<float> float_over { 0.0f, 1.0f, 2.0f };
    check_equal(float_over, 1.0f)

    BoundedProperty<float> float_valid { 0.0f, 1.0f, 0.5f };
    check_equal(float_valid, 0.5f);

    BoundedProperty<float> v(0.0f, 1.0f);
    check_equal(v, 0.0f);

    // Check setters
    v.set(10.0f);
    check_equal(v, 1.0f);
    v = 10.0f;
    check_equal(v, 1.0f);
    v.set(-10.0f);
    check_equal(v, 0.0f);
    v = -10.0f;
    check_equal(v, 0.0f);

    // Check that notifiers are called only when inside range
    bool called = false;
    v = 0.0f;
    v.connect([&]() { called = true; });

    v = 0.5f;
    check_equal(v, 0.5f);
    check_true(called);

    called = false;
    v = -1.0f;
    check_equal(v, 0.0f);
    check_true(called);

    called = false;
    v = -2.0f;
    check_equal(v, 0.0f);
    check_true(!called);

    called = false;
    v = 10.0f;
    check_equal(v, 1.0f);
    check_true(called);

    called = false;
    v = 100.0f;
    check_equal(v, 1.0f);
    check_true(!called);

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

int main(int argc, char **argv)
{
    tst_property();
    tst_bounded_property();
}