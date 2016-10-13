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

class LayoutEngine
{
public:
    enum LayoutType {
        Grid_Horizontal,
        Grid_Vertical,
        Flow_Horizontal,
        Flow_Vertical,
        LastLayoutType // ### Remove once we remove LayoutNode::setLayoutTypeAsInt()
        // packed in a bit field, expand when adding new members.
    };

    LayoutEngine()
        : margin(0.0f)
        , spacing(0.0f)
        , width(0.0f)
        , height(0.0f)
        , cellWidth(0.0f)
        , cellHeight(0.0f)
        , columnCount(0)
        , rowCount(0)
        , layoutType(Grid_Horizontal)
        , reserved1b_1(0) // corresponds to LayoutNode::Automatic
    {
    }

    inline void updateLayout(Node *parentNode);

    float margin;
    float spacing;
    float width;
    float height;
    float cellWidth;
    float cellHeight;
    int columnCount;
    int rowCount;

    unsigned layoutType : 2;
    unsigned reserved1b_1 : 1; // used by layout node to avoid having another 4 bytes to store activation mode..
};


class LayoutNode : public Node
{
public:
    enum ActivationMode {
        Automatic,
        Explicit
        // packed in a bit field, expand when adding new members.
    };

    RENGINE_ALLOCATION_POOL_DECLARATION(LayoutNode, rengine_LayoutNode);

    static Signal<> onMarginChanged;
    float margin() const { return m_engine.margin; }
    void setMargin(float margin) {
        if (margin == m_engine.margin)
            return;
        m_engine.margin = margin;
        onMarginChanged.emit(this);
    }

    static Signal<> onSpacingChanged;
    float spacing() const { return m_engine.spacing; }
    void setSpacing(float spacing) {
        if (spacing == m_engine.spacing)
            return;
        m_engine.spacing = spacing;
        onSpacingChanged.emit(this);
    }

    static Signal<> onWidthChanged;
    float width() const { return m_engine.width; }
    void setWidth(float width) {
        if (width == m_engine.width)
            return;
        m_engine.width = width;
        onWidthChanged.emit(this);
    }

    static Signal<> onHeightChanged;
    float height() const { return m_engine.height; }
    void setHeight(float height) {
        if (height == m_engine.height)
            return;
        m_engine.height = height;
        onHeightChanged.emit(this);
    }

    static Signal<> onCellWidthChanged;
    float cellWidth() const { return m_engine.cellWidth; }
    void setCellWidth(float cellWidth) {
        if (cellWidth == m_engine.cellWidth)
            return;
        m_engine.cellWidth = cellWidth;
        onCellWidthChanged.emit(this);
    }

    static Signal<> onCellHeightChanged;
    float cellHeight() const { return m_engine.cellHeight; }
    void setCellHeight(float cellHeight) {
        if (cellHeight == m_engine.cellHeight)
            return;
        m_engine.cellHeight = cellHeight;
        onCellHeightChanged.emit(this);
    }

    static Signal<> onRowCountChanged;
    int rowCount() const { return m_engine.rowCount; }
    void setRowCount(int rowCount) {
        if (rowCount == m_engine.rowCount)
            return;
        m_engine.rowCount = rowCount;
        onRowCountChanged.emit(this);
    }

    static Signal<> onColumnCountchanged;
    int columnCount() const { return m_engine.columnCount; }
    void setColumnCount(int columnCount) {
        if (columnCount == m_engine.columnCount)
            return;
        m_engine.columnCount = columnCount;
        onColumnCountchanged.emit(this);
    }

    ActivationMode activationMode() const { return (ActivationMode) m_engine.reserved1b_1; }
    void setActivationMode(ActivationMode mode) {
        if (mode == m_engine.reserved1b_1)
            return;
        m_engine.reserved1b_1 = mode;
        if (mode == Automatic)
            requestPreprocess();
    }

    static Signal<> onLayoutTypeChanged;
    LayoutEngine::LayoutType layoutType() const { return (LayoutEngine::LayoutType) m_engine.layoutType; }
    void setLayoutType(LayoutEngine::LayoutType type) {
        if (type == m_engine.layoutType)
            return;
        m_engine.layoutType = type;
        onLayoutTypeChanged.emit(this);
    }

    // ### Remove when code generation supports enums...
    static Signal<> onLayoutTypeAsIntChanged;
    int layoutTypeAsInt() const { return m_engine.layoutType; }
    void setLayoutTypeAsInt(int layoutType) {
        assert(layoutType >= 0 && layoutType < LayoutEngine::LastLayoutType);
        if (layoutType == m_engine.layoutType)
            return;
        setLayoutType((LayoutEngine::LayoutType) layoutType);
        onLayoutTypeAsIntChanged.emit(this);
    }

    void updateLayout() { m_engine.updateLayout(this); }

protected:
    LayoutNode()
    {
        requestPreprocess();
    }

    void onPreprocess() override {
        updateLayout();
        // Ensure that we get called again before the next frame..
        if (activationMode() == Automatic)
            requestPreprocess();
    }

    LayoutEngine m_engine;
};

void LayoutEngine::updateLayout(Node *parentNode)
{
    if (layoutType == Grid_Horizontal || layoutType == Grid_Vertical) {
        assert(cellWidth != 0 || (width != 0 && columnCount > 0));
        assert(cellHeight != 0 || (height != 0 && rowCount > 0));

        int index = 0;

        float cw, ch, xSign, ySign;

        if (cellHeight != 0) {
             cw = std::abs(cellWidth);
             xSign = cellWidth > 0 ? 1 : -1;
        } else {
            cw = (std::abs(width) - 2 * margin - (columnCount - 1) * spacing) / columnCount;
            xSign = width > 0 ? 1 : -1;
        }

        if (cellHeight != 0) {
            ch = std::abs(cellHeight);
            ySign = cellHeight < 0 ? -1 : 1;
        } else {
            ch = (std::abs(height) - 2 * margin - (rowCount - 1) * spacing) / rowCount;
            ySign = height < 0 ? -1 : 1;
        }

        logd << (layoutType == Grid_Horizontal ? "horizontal" : "vertical")
             << "-grid: cell=" << cw << "x" << ch
             << ", dim=" << width << "x" << height << ", margin=" << margin << ", spacing=" << spacing
             << std::endl;

        const int itemsPer = layoutType == Grid_Horizontal
                             ? (columnCount > 0 ? columnCount : std::numeric_limits<int>::max())
                             : (rowCount > 0 ? rowCount : std::numeric_limits<int>::max());
        Node *node = parentNode->child();
        while (node) {
            RectangleNodeBase *rectNode = RectangleNodeBase::from(node);
            if (rectNode) {
                int r, c;
                if (layoutType == Grid_Horizontal) {
                    r = index / itemsPer;
                    c = index % itemsPer;
                } else {
                    r = index % itemsPer;
                    c = index / itemsPer;
                }

                rectNode->setGeometry(rect2d::fromXywh(xSign * (margin + c * cw + c * spacing),
                                                       ySign * (margin + r * ch + r * spacing),
                                                       xSign * cw,
                                                       ySign * ch).normalized());
                ++index;
            }
            node = node->sibling();
        }

    } else {

        int index = 0;
        int itemLimit;
        float sizeLimit;

        float posSign, offsetSign;

        if (layoutType == Flow_Horizontal) {
            posSign = width < 0 ? -1 : 1;
            offsetSign = height < 0 ? -1 : 1;
            itemLimit = columnCount > 0 ? columnCount : std::numeric_limits<int>::max();
            sizeLimit = width != 0 ? width - posSign * margin * 2 : posSign * std::numeric_limits<float>::infinity();
        } else {
            posSign = height < 0 ? -1 : 1;
            offsetSign = width < 0 ? -1 : 1;
            itemLimit = rowCount > 0 ? rowCount : std::numeric_limits<int>::max();
            sizeLimit = height != 0 ? height - posSign * margin * 2 : posSign * std::numeric_limits<float>::infinity();
        }

        float pos = posSign * margin; // The position to place the node on the current line
        float offset = offsetSign * margin; // The position in the direction normal to the pos
        float offsetIncrement = 0;

        logd << (layoutType == Flow_Horizontal ? "horizontal" : "vertical")
             << "-flow: itemLimit=" << itemLimit << ", sizeLimit=" << sizeLimit
             << ", margin=" << margin << ", spacing=" << spacing
             << std::endl;

        Node *node = parentNode->child();
        while (node) {
            RectangleNodeBase *rectNode = RectangleNodeBase::from(node);
            if (rectNode) {
                float dim = layoutType == Flow_Horizontal ? rectNode->width() : rectNode->height();
                float end = pos + posSign * dim;
                bool fitsOnLine = (posSign * end <= posSign * sizeLimit && index < itemLimit);
                logd << " - pos=" << pos << ", dim=" << dim << ", end=" << end  << ", index=" << index << ", fitsOnLine=" << fitsOnLine << std::endl;

                if (fitsOnLine || index == 0) {
                    if (layoutType == Flow_Horizontal) {
                        offsetIncrement = std::max(offsetIncrement, rectNode->height());
                        rectNode->setPosition(posSign > 0 ? pos : pos - rectNode->width(),
                                              offsetSign > 0 ? offset : offset - rectNode->height());
                    } else {
                        offsetIncrement = std::max(offsetIncrement, rectNode->width());
                        rectNode->setPosition(offsetSign > 0 ? offset : offset - rectNode->width(),
                                              posSign > 0 ? pos : pos - rectNode->height());
                    }
                    ++index;
                    pos = end + posSign * spacing;
                    node = node->sibling();
                } else {
                    // If it doesn't fit, wrap to the next line..
                    offset += offsetSign * (offsetIncrement + spacing);
                    pos = posSign * margin;
                    index = 0;
                }
            } else {
                node = node->sibling();
            }
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
