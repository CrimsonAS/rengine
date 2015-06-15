#include "test.h"

class StaticRenderTest {
public:
    virtual Node *build() = 0;
    virtual bool check() = 0;

    bool checkPixel(int x, int y, const vec4 &expected, float errorMargin=0.01)
    {
        assert(x >= 0);
        assert(x < m_w);
        assert(y >= 0);
        assert(y < m_h);

        unsigned pixel = m_pixels[(m_h - y - 1) * m_w + x];
        cout << hex << pixel << endl;
        vec4 color = vec4((pixel & 0x000000ff) >> 0,
                          (pixel & 0x0000ff00) >> 8,
                          (pixel & 0x00ff0000) >> 16,
                          (pixel & 0xff000000) >> 24) / 255.0;

        if (abs(color.x - expected.x) > errorMargin
            || abs(color.y - expected.y) > errorMargin
            || abs(color.z - expected.z) > errorMargin
            || abs(color.w - expected.w) > errorMargin) {
            cout << "pixels differ: (" << x << "," << y << ")=" << color << "; expected=" << expected << endl;
            assert(false);
        }

        return true;
    }

    Surface *surface() const { return m_surface; }
    void setSurface(Surface *surface) { m_surface = surface; }

    void setPixels(int w, int h, unsigned *pixels) {
        m_w = w;
        m_h = h;
        m_pixels = pixels;
    }

    virtual const char *name() const = 0;

private:
    int m_w;
    int m_h;
    unsigned *m_pixels;
    Surface *m_surface;
};

class TestBase : public StandardSurfaceInterface
{
public:
    Node *update(Node *root) {
        if (root)
            delete root;

        m_currentTest = tests.front();
        tests.pop_front();

        m_currentTest->setSurface(surface());

        return m_currentTest->build();
    }

    void afterRender() override {
        vec2 size = surface()->size();
        unsigned *pixels = (unsigned *) malloc(size.x * size.y * sizeof(unsigned));

        bool ok = renderer()->readPixels(0, 0, size.x, size.y, (unsigned char *) pixels);
        check_true(ok);

        m_currentTest->setPixels(size.x, size.y, pixels);
        if (m_currentTest->check()) {
            cout << "testng: '" << m_currentTest->name() << "': ok" << endl;
        } else {
            cout << m_currentTest->name() << ": failed!" << endl;
        }

        if (tests.empty())
            Backend::get()->quit();
        else
            surface()->requestRender();
    }

    StaticRenderTest *m_currentTest;
    list<StaticRenderTest *> tests;
};

class RedGreenBlue : public StaticRenderTest
{
public:
    Node *build() override {
        Node *root = new Node();
        *root << new RectangleNode(rect2d::fromXywh(0, 0, 1, 1), vec4(1, 0, 0, 1))
              << new RectangleNode(rect2d::fromXywh(1, 0, 1, 1), vec4(0, 1, 0, 1))
              << new RectangleNode(rect2d::fromXywh(2, 0, 1, 1), vec4(0, 0, 1, 1));
        return root;
    }

    bool check() override {
        return checkPixel(0, 0, vec4(1, 0, 0, 1))
            && checkPixel(1, 0, vec4(0, 1, 0, 1))
            && checkPixel(2, 0, vec4(0, 0, 1, 1));
    }

    const char *name() const override { return "RedGreenBlue"; }
};


int main(int argc, char *argv[])
{
    std::unique_ptr<Backend> backend(Backend::get());
    TestBase testBase;
    testBase.tests.push_back(new RedGreenBlue());
    Surface *surface = backend->createSurface(&testBase);
    surface->show();
    backend->run();
    return 0;
}