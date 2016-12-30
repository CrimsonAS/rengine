#include "examples.h"

#define  STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>


class TextExample : public StandardSurface
{
public:
    TextExample()
    {
    }

    void initialize();

    Node *update(Node *old) {
        if (old)
            return old;

        initialize();

        renderer()->setFillColor(vec4(0.0, 0.0, 0.0, 1));

        return old;
    }

    GlyphContext *m_font = nullptr;
    std::vector<shared_ptr<WorkQueue::Job> > m_jobs;
};

void TextExample::initialize()
{
    m_font = new GlyphContext("OpenSans-Regular.ttf");
    assert(m_font->isValid());

    Units units(this);

    shared_ptr<WorkQueue::Job> job(new GlyphTextureJob(m_font, "Hello", units.font()));
    m_jobs.push_back(job);

    for (auto job : m_jobs)
        workQueue()->schedule(job);
}

RENGINE_MAIN(TextExample)