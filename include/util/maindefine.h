#pragma once

RENGINE_BEGIN_NAMESPACE

template <typename InterfaceName>
int rengine_main(int argc, char **argv) {
    Backend *backend = Backend::get();
    InterfaceName iface;
    Surface *surface = backend->createSurface(&iface);
    surface->show();
    backend->run();
    return 0;
}

RENGINE_END_NAMESPACE

#define RENGINE_MAIN(InterfaceName)                     \
int main(int argc, char **argv) {                       \
    return RENGINE_NAMESPACE_PREFIX rengine_main<InterfaceName>(argc, argv);     \
}
