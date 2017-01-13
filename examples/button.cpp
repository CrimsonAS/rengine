#include "button.h"

using namespace rengine;

#define BG_BASE_COLOR vec4(0.2, 0.2, 0.3, 1.0)
#define BG_HOVER_COLOR vec4(0.3, 0.3, 0.4, 1.0)
#define BG_CLICK_COLOR vec4(0.4, 0.4, 0.5, 1.0)

Button::Button()
    : RectangleNodeBase(RectangleNodeBaseType)
{
    setPointerTarget(true);

    // We use this to perform the layout of the subtree
    requestPreprocess();

    m_xformNode = TransformNode::create();
    m_xformNode->setProjectionDepth(1000);
    m_bgNode = RectangleNode::create(rect2d(), BG_BASE_COLOR);
    m_textureNode = TextureNode::create(rect2d(), nullptr);

    *this
        << &(*m_xformNode
            << m_bgNode
            << m_textureNode
            );
}

void Button::onPreprocess()
{
    rect2d g = geometry();

    vec2 center = g.center();
    vec2 size = g.size();

    m_xformNode->setMatrix(mat4::translate2D(center));

    rect2d lg = rect2d::fromPosSize(-size / 2.0f, size);
    m_bgNode->setGeometry(lg);

    if (m_texture) {
        vec2 ts = m_texture->size();
        m_textureNode->setTexture(m_texture);
        m_textureNode->setGeometry(rect2d::fromPosSize(-ts / 2.0f, ts));
    }

}