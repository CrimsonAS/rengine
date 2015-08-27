#include "test.h"

bool cfunc_void_called = false;
static void cfunc_void()
{
    cfunc_void_called = true;
}

bool cfunc_int_called = false;
int cfuncvalue_int = 0;
static void cfunc_int(int x)
{
    cfunc_int_called = true;
    cfuncvalue_int = x;
}

bool cfunc_string_float_float_called = false;
std::string cfuncvalue_string;
float cfuncvalue_float1 = 0;
float cfuncvalue_float2 = 0;
static void cfunc_string_float_float(std::string s, float a, float b)
{
    cfunc_string_float_float_called = true;
    cfuncvalue_string = s;
    cfuncvalue_float1 = a;
    cfuncvalue_float2 = b;
}

void tst_signal_basic()
{
    bool called_void = false;
    bool called_int = false;
    bool called_string_float_float = false;

    Signal<> signal_void;
    signal_void.connect([&]() { called_void = true; });
    signal_void.connect(cfunc_void);
    signal_void.emit();
    check_true(cfunc_void_called);
    check_true(called_void);

    int emitted_int = 0;
    Signal<int> signal_int;
    signal_int.connect([&] (int value) {
        called_int = true;
        emitted_int = value;
    });
    signal_int.connect(cfunc_int);
    signal_int.emit(42);
    check_true(called_int);
    check_true(cfunc_int_called);
    check_equal(emitted_int, 42);
    check_equal(cfuncvalue_int, 42);

    std::string emitted_string;
    float emitted_float1 = 0;
    float emitted_float2 = 0;
    Signal<std::string, float, float> signal_string_float_float;
    signal_string_float_float.connect([&](std::string str, float a, float b) {
        called_string_float_float = true;
        emitted_string = str;
        emitted_float1 = a;
        emitted_float2 = b;
    });
    signal_string_float_float.connect(cfunc_string_float_float);
    signal_string_float_float.emit(std::string("one"), 2.0f, 3.0f);
    check_true(called_string_float_float);
    check_true(cfunc_string_float_float_called);
    check_equal(emitted_string, std::string("one"));
    check_equal(emitted_float1, 2.0f);
    check_equal(emitted_float2, 3.0f);
    check_equal(cfuncvalue_string, std::string("one"));
    check_equal(cfuncvalue_float1, 2.0f);
    check_equal(cfuncvalue_float2, 3.0f);

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

int main(int argc, char **argv)
{
    tst_signal_basic();
}