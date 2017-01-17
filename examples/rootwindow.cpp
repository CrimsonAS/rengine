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

#include "example.h"
#include "rectangles.h"

using namespace rengine;
using namespace std;

Node *RootWindow::build()
{
    const char *fontFile = "OpenSans-Regular.ttf";
    m_font = new GlyphContext(fontFile);
    if (!m_font->isValid()) {
        cerr << "Failed to load '" << fontFile << "'..." << endl
             << "The font file needs to be downloaded separately and placed next" << endl
             << "to the 'demo' binary." << endl;
    }
    assert(m_font->isValid());

    Units units(this);
    vec2 windowSize = size();

    Node *root = Node::create();

    // Prepare the app layer
    m_appLayer = OpacityNode::create();


    // Prepare the button grid
    m_buttonGrid = LayoutNode::create();
    m_buttonGrid->setActivationMode(LayoutNode::Explicit);
    m_buttonGrid->setLayoutType(LayoutEngine::Grid_Horizontal);
    m_buttonGrid->setCellWidth(units.base() * 15.0f);
    m_buttonGrid->setCellHeight(units.base() * 3.0f);
    m_buttonGrid->setSpacing(units.base() * 1.0f);
    m_buttonGrid->setMargin(units.base() * 1.0f);
    m_buttonGrid->setColumnCount(1);
    m_buttonGrid->setX(windowSize.x / 2 - m_buttonGrid->cellWidth() / 2);

    add(new Rectangles(), units);
    add(new Rectangles(), units);
    add(new Rectangles(), units);
    add(new Rectangles(), units);
    add(new Rectangles(), units);
    add(new Rectangles(), units);
    add(new Rectangles(), units);

    m_buttonGrid->updateLayout();

    root->append(m_buttonGrid);
    root->append(m_appLayer);

    // The close button..
    SignalHandler_Function<> *handler = new SignalHandler_Function<>([this] () { closeExample(); });
    m_buttonSignalHandlers.push_back(handler);
    m_closeButton = createTextButton("Close", units);
    m_closeButton->setGeometry(rect2d::fromXywh(m_buttonGrid->margin(), m_buttonGrid->margin(),
                                                m_buttonGrid->cellWidth(), m_buttonGrid->cellHeight()));

    Button::onClicked.connect(m_closeButton, handler);

    return root;
}

void RootWindow::animateButtons(Example *example, float from, float to, float delay)
{
    int pos = std::find(m_examples.begin(), m_examples.end(), example) - m_examples.begin();
    Button *button = static_cast<Button *>(m_buttonGrid->child());
    for (int i=0; i<m_examples.size(); ++i) {
        assert(button);
        int distance = abs(pos - i);
        button->scheduleRotation(from, to, 0.3, delay + distance * 0.05);
        button = static_cast<Button *>(button->sibling());
    }
}

void RootWindow::startExample(Example *example)
{
    assert(example);
    if (m_example) {
        cout << "RootWindow::startExample: '" << m_example->name()
        << "' is already running when asked to start '" << example->name() << "'" << endl;
        return;
    }

    animateButtons(example, 0, -M_PI / 2.0f, 0.0f);

    m_example = example;
    m_example->initialize();

    m_appLayer->setOpacity(0);
    m_appLayer->append(example);
    m_appLayer->append(m_closeButton);

    auto fade = std::make_shared<Animation_OpacityNode_opacity>(m_appLayer);
    fade->setDuration(0.5);
    fade->newKeyFrame(0) = 0;
    fade->newKeyFrame(1) = 1;

    animationManager()->start(fade, 0.5);
}

void RootWindow::closeExample()
{
    assert(m_example);

    animateButtons(m_example, M_PI / 2.0f, 0, 0.3);

    std::vector<std::shared_ptr<AbstractAnimation>> sequence;

    auto fade = std::make_shared<Animation_OpacityNode_opacity>(m_appLayer);
    fade->setDuration(0.5);
    fade->newKeyFrame(0) = 1;
    fade->newKeyFrame(1) = 0;

    auto close = std::make_shared<Animation_Callback<RootWindow, &RootWindow::exampleClosed>>(this);

    sequence.push_back(fade);
    sequence.push_back(close);

    animationManager()->start(sequence, 0);
}

void RootWindow::exampleClosed()
{
    assert(m_example);
    setPointerEventReceiver(nullptr);
    m_appLayer->remove(m_example);
    m_appLayer->remove(m_closeButton);
    m_appLayer->setOpacity(0);
    m_example->invalidate();
    m_example = 0;
}


Button *RootWindow::createTextButton(const char *text, const Units &units)
{
   // Perform the job synchronously because this is part of the initial ui.
    GlyphTextureJob job(m_font, text, units.font());
    job.onExecute();
    assert(job.textureSize().x > 0 && job.textureSize().y > 0);
    Texture *t = renderer()->createTextureFromImageData(job.textureSize(), Texture::RGBA_32, job.textureData());

    Button *button = new Button(this);
    button->setTextTexture(t);

    return button;
}

void RootWindow::add(Example *example, const Units &units)
{
    example->setWindow(this);

    SignalHandler_Function<> *handler = new SignalHandler_Function<>([this, example] () { startExample(example); });
    m_buttonSignalHandlers.push_back(handler);

    Button *button = createTextButton(example->name(), units);
    button->scheduleRotation(M_PI / 2.0f, 0, 0.8, m_examples.size() * 0.1);
    Button::onClicked.connect(button, handler);

    m_buttonGrid->append(button);

    m_examples.push_back(example);
}

