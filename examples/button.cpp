#include "button.h"

using namespace rengine;

#define BG_BASE_COLOR vec4(0.2, 0.2, 0.3, 1.0)
#define BG_HOVER_COLOR vec4(0.3, 0.3, 0.4, 1.0)
#define BG_CLICK_COLOR vec4(0.4, 0.4, 0.5, 1.0)

Signal<> Button::onClicked;

Button::Button(StandardSurface *surface)
    : RectangleNodeBase(RectangleNodeBaseType)
    , m_surface(surface)
    , m_xformNode(TransformNode::create())
    , m_offsetNode(TransformNode::create())
    , m_bgNode(RectangleNode::create(rect2d(), BG_BASE_COLOR))
    , m_textureNode(TextureNode::create(rect2d(), nullptr))
    , m_animation(m_xformNode)
{
    setPointerTarget(true);

    // We use this to perform the layout of the subtree
    requestPreprocess();

    m_xformNode->setProjectionDepth(1000);

    m_animation.setDuration(1);
    m_animation.setIterations(1);
    m_animation.keyFrames.times() << 0 << 1;

    *this
        << &(*m_offsetNode
            << &(*m_xformNode
                << m_bgNode
                << m_textureNode
                )
            );
}

Button::~Button()
{
    m_surface->animationManager()->stopAnimation(&m_animation);
}

void Button::onPreprocess()
{
    rect2d g = geometry();

    vec2 center = g.center();
    vec2 size = g.size();

    m_offsetNode->setMatrix(mat4::translate2D(center));

    rect2d lg = rect2d::fromPosSize(-size / 2.0f, size);
    m_bgNode->setGeometry(lg);

    if (m_texture) {
        vec2 ts = m_texture->size();
        m_textureNode->setTexture(m_texture.get());
        m_textureNode->setGeometry(rect2d::fromPosSize(-ts / 2.0f, ts));
    }

    if (m_state == PressedState)
        m_bgNode->setColor(BG_CLICK_COLOR);
    else if (m_state == HoveredState)
        m_bgNode->setColor(BG_HOVER_COLOR);
    else
        m_bgNode->setColor(BG_BASE_COLOR);
}

void Button::setTextTexture(Texture *tex)
{
    if (m_texture.get() == tex)
        return;
    m_texture.reset(tex);
    requestPreprocess();
}

void Button::scheduleRotation(float from, float to, float time, float delay)
{
    m_xformNode->setMatrix(mat4::rotateAroundY(from));
    m_animation.setDuration(time);
    m_animation.keyFrames = KeyFrames<TransformNode>();
    m_animation.keyFrames.times() << 0 << 1;
    m_animation.keyFrames.addValues<double, TransformNode_rotateAroundY>() << from << to;
    m_surface->animationManager()->scheduleAnimation(delay, &m_animation);
}

bool Button::onPointerEvent(PointerEvent *e)
{
    if (e->type() == Event::PointerDown) {
        setState(PressedState);
    } else if (e->type() == Event::PointerUp) {
        setState(HoveredState);
        onClicked.emit(this);
    } else if (e->type() == Event::PointerMove) {
        if (geometry().contains(e->position())) {
            m_surface->setPointerEventReceiver(this);
            setState(HoveredState);
        } else {
            m_surface->setPointerEventReceiver(nullptr);
            setState(DefaultState);
            return false;
        }
    }

    return true;
}

void Button::setState(State state)
{
    if (state == m_state)
        return;
    m_state = state;
    requestPreprocess();
    m_surface->requestRender();

}