/*
    Copyright (c) 2017, Gunnar Sletta <gunnar@crimson.no>
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
*/

#include "rootwindow.h"
#include "button.h"

using namespace rengine;

Node *RootWindow::build()
{
    const char *fontFile = "OpenSans-Regular.ttf";
    m_font = new GlyphContext(fontFile);
    if (!m_font->isValid()) {
        std::cerr << "Failed to load '" << fontFile << "'..." << std::endl
                  << "The font file needs to be downloaded separately and placed next" << std::endl
                  << "to the 'demo' binary." << std::endl;
    }
    assert(m_font->isValid());

    Units units(this);

    Node *root = Node::create();
    // m_appLayer = Node::create();

    m_buttonGrid = LayoutNode::create();
    m_buttonGrid->setActivationMode(LayoutNode::Explicit);
    m_buttonGrid->setLayoutType(LayoutEngine::Grid_Horizontal);
    m_buttonGrid->setCellWidth(units.base() * 15.0f);
    m_buttonGrid->setCellHeight(units.base() * 3.0f);
    m_buttonGrid->setSpacing(units.base() * 1.0f);
    // m_buttonGrid->setWidth(s.x);
    // m_buttonGrid->setHeight(s.y);
    m_buttonGrid->setMargin(units.base() * 1.0f);
    m_buttonGrid->setColumnCount(1);

    add("Rectangles", nullptr, units);
    add("Blur", nullptr, units);
    add("Shadow", nullptr, units);
    add("Color Filters", nullptr, units);
    add("Layered Opacity", nullptr, units);
    add("Fonts & Text", nullptr, units);
    // add("Blurring", nullptr);
    // add("Blending Benchmark", nullptr);

    m_buttonGrid->updateLayout();

    root->append(m_buttonGrid);

    return root;
}

void RootWindow::add(const char *title, ExampleNode *example, const Units &units)
{
    Button *button = new Button(this);
    m_buttonGrid->append(button);

    // Perform the job synchronously because this is part of the initial ui.
    GlyphTextureJob job(m_font, title, units.font());
    job.onExecute();
    assert(job.textureSize().x > 0 && job.textureSize().y > 0);
    Texture *t = renderer()->createTextureFromImageData(job.textureSize(), Texture::RGBA_32, job.textureData());

    button->setTextTexture(t);
    button->scheduleRotation(M_PI / 2.0f, 0, 1.0, m_buttons.size() * 0.2);

    m_buttons.push_back(button);
}


void RootWindow::onEvent(Event *e)
{
    switch (e->type()) {
    case Event::PointerDown:
    case Event::PointerUp:
    case Event::PointerMove:
        if (renderer() && renderer()->sceneRoot()) {
            PointerEvent *pe = PointerEvent::from(e);
            if (!deliverPointerEventInScene(m_renderer->sceneRoot(), pe)) {
                updateHoverTarget(nullptr);
            }
        }
        break;
    default:
        std::cerr << __PRETTY_FUNCTION__ << ": unknown event type=" << e->type() << std::endl;
        break;
    }
}

bool RootWindow::onPointerEvent(Node *node, PointerEvent *e)
{
    updateHoverTarget(node);
    return true;
}

void RootWindow::updateHoverTarget(Node *node)
{
    if (m_hoverTarget != node) {
        if (m_hoverTarget)
            m_hoverTarget->setPointerOver(false);
        m_hoverTarget = node;
        if (m_hoverTarget)
            m_hoverTarget->setPointerOver(true);
        requestRender();
    }
}
