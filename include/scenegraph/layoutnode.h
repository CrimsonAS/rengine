/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
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

#pragma once

#include <limits>

RENGINE_BEGIN_NAMESPACE

/*!

 */

class LayoutNode : public Node
{
public:
    enum LayoutType {
        Grid_Horizontal,
        Grid_Vertical,
        Flow_Horizontal,
        Flow_Vertical
        // packed in a bit field, expand when adding new members.
    };

    enum ActivationMode {
        Automatic,
        Explicit
        // packed in a bit field, expand when adding new members.
    };

    RENGINE_ALLOCATION_POOL_DECLARATION(LayoutNode, rengine_LayoutNode);

    RENGINE_IMPLEMENT_PROPERTY(float, margin, setMargin, onMarginChanged);
    RENGINE_IMPLEMENT_PROPERTY(float, spacing, setSpacing, onSpacingChanged);
    RENGINE_IMPLEMENT_PROPERTY(float, width, setWidth, onWidthChanged);
    RENGINE_IMPLEMENT_PROPERTY(float, height, setHeight, onHeightChanged);

    // Only used by grid..
    RENGINE_IMPLEMENT_PROPERTY(float, cellWidth, setCellWidth, onCellWidthChanged);
    RENGINE_IMPLEMENT_PROPERTY(float, cellHeight, setCellHeight, onCellHeightChanged);
    RENGINE_IMPLEMENT_PROPERTY(int, columnCount, setColumnCount, onColumnCountchanged);
    RENGINE_IMPLEMENT_PROPERTY(int, rowCount, setRowCount, onRowCountChanged);

    ActivationMode activationMode() const { return m_activationMode; }

    void setActivationMode(ActivationMode mode) {
        if (mode == m_activationMode)
            return;
        m_activationMode = mode;
        if (m_activationMode == Automatic)
            requestPreprocess();
    }

    static Signal<> onLayoutTypeChanged;
    LayoutType layoutType() const { return m_layoutType; }
    void setLayoutType(LayoutType type) {
        if (type == m_layoutType)
            return;
        m_layoutType = type;
        onLayoutTypeChanged.emit(this);
    }

    // ### Remove when code generation supports enums...
    static Signal<> onLayoutTypeAsIntChanged;
    int layoutTypeAsInt() const { return m_layoutType; }
    void setLayoutTypeAsInt(int layoutType) {
        assert(layoutType >= 0 && layoutType <= Flow_Vertical);
        setLayoutType((LayoutType) layoutType);
    }

    void updateLayout();


protected:
    LayoutNode()
        : m_activationMode(Automatic)
        , m_layoutType(Grid_Horizontal)
    {
        m_columnCount = 0;
        m_rowCount = 0;
        m_cellWidth = 0;
        m_cellHeight = 0;
        m_width = 0;
        m_height = 0;
        m_margin = 0;
        m_spacing = 0;

        requestPreprocess();
    }

    void onPreprocess() override {
        updateLayout();
        // Ensure that we get called again before the next frame..
        if (m_activationMode == Automatic)
            requestPreprocess();
    }

    ActivationMode m_activationMode : 1;
    LayoutType m_layoutType : 2;
};

void LayoutNode::updateLayout()
{
    if (m_layoutType == Grid_Horizontal || m_layoutType == Grid_Vertical) {
        assert(m_cellWidth != 0 || (m_width != 0 && m_columnCount > 0));
        assert(m_cellHeight != 0 || (m_height != 0 && m_rowCount > 0));

        int index = 0;

        float cellWidth, cellHeight, xSign, ySign;

        if (m_cellHeight != 0) {
             cellWidth = std::abs(m_cellWidth);
             xSign = m_cellWidth > 0 ? 1 : -1;
        } else {
            cellWidth = (std::abs(m_width) - 2 * m_margin - (m_columnCount - 1) * m_spacing) / m_columnCount;
            xSign = m_width > 0 ? 1 : -1;
        }

        if (m_cellHeight != 0) {
            cellHeight = std::abs(m_cellHeight);
            ySign = m_cellHeight > 0 ? 1 : -1;
        } else {
            cellHeight = (std::abs(m_height) - 2 * m_margin - (m_rowCount - 1) * m_spacing) / m_rowCount;
            ySign = m_height > 0 ? 1 : -1;
        }

        logd << (m_layoutType == Grid_Horizontal ? "horizontal" : "vertical")
             << "-grid: cell=" << cellWidth << "x" << cellHeight
             << ", dim=" << m_width << "x" << m_height << ", margin=" << m_margin << ", spacing=" << m_spacing
             << std::endl;

        const int itemsPer = m_layoutType == Grid_Horizontal
                             ? (m_columnCount > 0 ? m_columnCount : 1)
                             : (m_rowCount > 0 ? m_rowCount : 1);
        Node *node = child();
        while (node) {
            RectangleNodeBase *rectNode = RectangleNodeBase::from(node);
            if (rectNode) {
                int r, c;
                if (m_layoutType == Grid_Horizontal) {
                    r = index / itemsPer;
                    c = index % itemsPer;
                } else {
                    r = index % itemsPer;
                    c = index / itemsPer;
                }

                rectNode->setGeometry(rect2d::fromXywh(xSign * (m_margin + c * cellWidth + c * m_spacing),
                                                       ySign * (m_margin + r * cellHeight + r * m_spacing),
                                                       xSign * cellWidth,
                                                       ySign * cellHeight).normalized());
                ++index;
            }
            node = node->sibling();
        }

    } else {

        int index = 0;
        int itemLimit;
        float sizeLimit;

        if (m_layoutType == Flow_Horizontal) {
            itemLimit = m_columnCount > 0 ? m_columnCount : std::numeric_limits<int>::max();
            sizeLimit = m_width != 0 ? m_width - m_margin : std::numeric_limits<float>::infinity();
        } else {
            itemLimit = m_rowCount > 0 ? m_rowCount : std::numeric_limits<int>::max();
            sizeLimit = m_height != 0 ? m_height - m_margin : std::numeric_limits<float>::infinity();
        }

        float flow = m_margin; // The position in the direction of the flow
        float step = m_margin; // The position in the direction normal to the flow
        float stepIncrement = 0;

        logd << (m_layoutType == Flow_Horizontal ? "horizontal" : "vertical")
             << "-flow: itemLimit=" << itemLimit << ", sizeLimit=" << sizeLimit
             << ", margin=" << m_margin << ", spacing=" << m_spacing
             << std::endl;

        Node *node = child();
        while (node) {
            RectangleNodeBase *rectNode = RectangleNodeBase::from(node);
            if (rectNode) {
                float dim = m_layoutType == Flow_Horizontal ? rectNode->width() : rectNode->height();
                float end = flow + dim;
                std::cout << " - " << rectNode->geometry() << " index=" << index << ", flow=" << flow << ", step=" << step << ", end=" << end << " - " << rectNode->width();
                if (index == 0 || (end < sizeLimit && index < itemLimit)) {
                    std::cout << " - inside - ";
                    index++;
                    if (m_layoutType == Flow_Horizontal) {
                        stepIncrement = std::max(stepIncrement, rectNode->height());
                        rectNode->setPosition(flow, step);
                    } else {
                        stepIncrement = std::max(stepIncrement, rectNode->width());
                        rectNode->setPosition(step, flow);
                    }
                    flow = end + m_spacing;
                } else {
                    std::cout << " - outside - ";
                    index = 0;
                    step += stepIncrement + m_spacing;
                    stepIncrement = 0;
                    flow = m_margin + dim + m_spacing;
                    if (m_layoutType == Flow_Horizontal)
                        rectNode->setPosition(m_margin, step);
                    else
                        rectNode->setPosition(step, m_margin);
                }
                std::cout << rectNode->geometry() << std::endl;
            }

            node = node->sibling();
        }

    }
}

#define RENGINE_LAYOUTNODE_DEFINE_SIGNALS                              \
    rengine::Signal<> rengine::LayoutNode::onMarginChanged;            \
    rengine::Signal<> rengine::LayoutNode::onSpacingChanged;           \
    rengine::Signal<> rengine::LayoutNode::onWidthChanged;             \
    rengine::Signal<> rengine::LayoutNode::onHeightChanged;            \
    rengine::Signal<> rengine::LayoutNode::onCellWidthChanged;         \
    rengine::Signal<> rengine::LayoutNode::onCellHeightChanged;        \
    rengine::Signal<> rengine::LayoutNode::onColumnCountchanged;       \
    rengine::Signal<> rengine::LayoutNode::onRowCountChanged;          \
    rengine::Signal<> rengine::LayoutNode::onLayoutTypeChanged;        \
    rengine::Signal<> rengine::LayoutNode::onLayoutTypeAsIntChanged;


#define RENGINE_LAYOUTNODE_DEFINE_ALLOCATION_POOLS \
    RENGINE_ALLOCATION_POOL_DEFINITION(rengine::LayoutNode, rengine_LayoutNode);


RENGINE_END_NAMESPACE