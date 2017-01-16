#include "examples.h"

class TextExample : public StandardSurface
{
public:
    TextExample()
    {
    }

    void initialize();

    void onEvent(Event *event) override
    {
        std::cout << "event..." << std::endl;

        if (event->type() == Event::PointerDown) {
            static bool on = true;
            if (on) {
                glEnable(GL_FRAMEBUFFER_SRGB);
            } else {
                glDisable(GL_FRAMEBUFFER_SRGB);
            }
            on = !on;
            requestRender();
        }
    }

    Node *build() override {
        initialize();
        renderer()->setFillColor(vec4(0.0, 0.0, 0.0, 1));
        requestRender();
        Node *root = Node::create();
        // root->append(RectangleNode::create(rect2d::fromXywh(50, 0, 50, 1000), vec4(1, 1, 1, 1)));
        return root;
    }

    Node *update(Node *root) override {

        cout << "running an update..." << endl;

        while (m_jobs.size() && m_jobs.front()->hasCompleted()) {
            std::cout << " - fetching one from the queue..." << endl;

            shared_ptr<WorkQueue::Job> job = m_jobs.front();
            m_jobs.pop_front();

            GlyphTextureJob *glyphJob = static_cast<GlyphTextureJob *>(job.get());
            Texture *texture = renderer()->createTextureFromImageData(glyphJob->textureSize(), Texture::RGBA_32, glyphJob->textureData());
            TextureNode *tn = TextureNode::create(rect2d::fromXywh(10, 10 + offset, texture->size().x, texture->size().y), texture);

            offset += texture->size().y + 10;

            std::cout << tn << std::endl;

            root->append(tn);
        }

        if (m_jobs.size()) {
            std::cout << " - more pending, scheduleing an update..." << endl;
            requestRender();
        }

        return root;
    }

    int offset = 0;

    GlyphContext *m_font = nullptr;
    std::list<shared_ptr<WorkQueue::Job> > m_jobs;
};

void TextExample::initialize()
{
    m_font = new GlyphContext("OpenSans-Regular.ttf");
    assert(m_font->isValid());

    Units units(this);

    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'tinyFont'", units.tinyFont())));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'smallFont'", units.smallFont())));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'font'", units.font())));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'largeFont'", units.largeFont())));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'hugeFont'", units.hugeFont())));

    // vec4 black(0, 0, 0, 1);
    // m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'tinyFont'", units.tinyFont(), black)));
    // m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'smallFont'", units.smallFont(), black)));
    // m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'font'", units.font(), black)));
    // m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'largeFont'", units.largeFont(), black)));
    // m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'hugeFont'", units.hugeFont(), black)));

    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'red'",     units.font(), vec4(1, 0, 0, 1))));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'green'",   units.font(), vec4(0, 1, 0, 1))));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'blue'",    units.font(), vec4(0, 0, 1, 1))));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'yellow'",  units.font(), vec4(1, 1, 0, 1))));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'cyan'",    units.font(), vec4(0, 1, 1, 1))));
    m_jobs.push_back(shared_ptr<WorkQueue::Job>(new GlyphTextureJob(m_font, "Open Sans Regular, 'magenta'", units.font(), vec4(1, 0, 1, 1))));

    for (auto job : m_jobs)
        workQueue()->schedule(job);
}

RENGINE_MAIN(TextExample)