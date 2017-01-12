#include "test.h"

static Surface *surface;

void tst_dpi()
{
    Units units(surface);

    cout << "Units::uiBase ....: " << units.base() << endl;
    cout << "Units::tinyFont ..: " << units.tinyFont() << endl;
    cout << "Units::smallFont .: " << units.smallFont() << endl;
    cout << "Units::font ......: " << units.font() << endl;
    cout << "Units::largeFont .: " << units.largeFont() << endl;
    cout << "Units::hugeFont ..: " << units.hugeFont() << endl;
    cout << "Units::mm ........: " << units.mm() << endl;
    cout << "Units::cm ........: " << units.cm() << endl;
    cout << "Units::inch ......: " << units.inch() << endl;

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

int main(int argc, char **argv)
{
    std::unique_ptr<rengine::Backend> backend(rengine::Backend::get());
    surface = new Surface();

    tst_dpi();

    return 0;
}