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

#include "test.h"


void tst_layoutnode_properties()
{
    int callCounter = 0;
    SignalHandler_Function<> signalHandler([&] {
        callCounter++;
    });

    LayoutNode *node = LayoutNode::create();

    LayoutNode::onMarginChanged.connect(node, &signalHandler);
    node->setMargin(10);
    LayoutNode::onMarginChanged.disconnect(node, &signalHandler);
    check_equal(node->margin(), 10.0f);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onSpacingChanged.connect(node, &signalHandler);
    node->setSpacing(20);
    LayoutNode::onSpacingChanged.disconnect(node, &signalHandler);
    check_equal(node->spacing(), 20.0f);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onWidthChanged.connect(node, &signalHandler);
    node->setWidth(30);
    LayoutNode::onWidthChanged.disconnect(node, &signalHandler);
    check_equal(node->width(), 30.0f);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onHeightChanged.connect(node, &signalHandler);
    node->setHeight(40);
    LayoutNode::onHeightChanged.disconnect(node, &signalHandler);
    check_equal(node->height(), 40.0f);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onCellWidthChanged.connect(node, &signalHandler);
    node->setCellWidth(50);
    LayoutNode::onCellWidthChanged.disconnect(node, &signalHandler);
    check_equal(node->cellWidth(), 50.0f);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onCellHeightChanged.connect(node, &signalHandler);
    node->setCellHeight(60);
    LayoutNode::onCellHeightChanged.disconnect(node, &signalHandler);
    check_equal(node->cellHeight(), 60.0f);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onColumnCountchanged.connect(node, &signalHandler);
    node->setColumnCount(70);
    LayoutNode::onColumnCountchanged.disconnect(node, &signalHandler);
    check_equal(node->columnCount(), 70)
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onRowCountChanged.connect(node, &signalHandler);
    node->setRowCount(80);
    LayoutNode::onRowCountChanged.disconnect(node, &signalHandler);
    check_equal(node->rowCount(), 80);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onLayoutTypeChanged.connect(node, &signalHandler);
    node->setLayoutType(LayoutEngine::Flow_Vertical);
    LayoutNode::onLayoutTypeChanged.disconnect(node, &signalHandler);
    check_equal(node->layoutType(), LayoutEngine::Flow_Vertical);
    check_equal(callCounter, 1);

    callCounter = 0;
    LayoutNode::onLayoutTypeAsIntChanged.connect(node, &signalHandler);
    node->setLayoutTypeAsInt(LayoutEngine::Flow_Horizontal);
    LayoutNode::onLayoutTypeAsIntChanged.disconnect(node, &signalHandler);
    check_equal(node->layoutTypeAsInt(), LayoutEngine::Flow_Horizontal);
    check_equal(callCounter, 1);

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

static void apply_w_h_cw_ch_r_c_m_s(LayoutNode *node, float width, float height, float cellWidth, float cellHeight, int rowCount, int columnCount, float margin, float spacing)
{
    node->setWidth(width);
    node->setHeight(height);
    node->setCellWidth(cellWidth);
    node->setCellHeight(cellHeight);
    node->setRowCount(rowCount);
    node->setColumnCount(columnCount);
    node->setSpacing(spacing);
    node->setMargin(margin);

    node->updateLayout();
}


void tst_layoutnode_horizontal_grid()
{
    LayoutNode *root = LayoutNode::create();
    root->setActivationMode(LayoutNode::Explicit);
    root->setLayoutType(LayoutEngine::Grid_Horizontal);

    RectangleNode *r[4];
    for (int i=0; i<4; ++i) {
        r[i] = RectangleNode::create();
        root->append(r[i]);
    }

    // Layout out in a single row, based on cell size
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, 0, 10, 20));
    // Layout out in a single row, negative cell width
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, 0, 10, 20));
    // Layout out in a single row, negative cell dimensions
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, -20, 10, 20));
    // Layout out in a single row, negative cell height
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, -20, 10, 20));

    // Layout out in a single row, based on cell size. With margin/spacing
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, 1, 10, 20));
    // Layout out in a single row, negative cellwidth, with margin/spacing
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, 1, 10, 20));
    // Layout out in a single row, negative cell dimensions, with margin/spacing
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, -21, 10, 20));
    // Layout out in a single row, negative cell height, with margin/spacing
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, -21, 10, 20));

    // Layout out in a single row, based on width/4cols
    apply_w_h_cw_ch_r_c_m_s(root, 40, 20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, 0, 10, 20));
    // Layout out in a single row, negative width/4cols
    apply_w_h_cw_ch_r_c_m_s(root, -40, 20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, 0, 10, 20));
    // Layout out in a single row, negative width/4cols negative height
    apply_w_h_cw_ch_r_c_m_s(root, -40, -20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, -20, 10, 20));
    // Layout out in a single row, width/4cols and negative height
    apply_w_h_cw_ch_r_c_m_s(root, 40, -20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, -20, 10, 20));

    // Layout out in a single row, based on width/4cols + margin/space
    apply_w_h_cw_ch_r_c_m_s(root, 48, 22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, 1, 10, 20));
    // Layout out in a single row, negative width/4cols + margin/space
    apply_w_h_cw_ch_r_c_m_s(root, -48, 22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, 1, 10, 20));
    // Layout out in a single row, negative width/4cols negative height + margin/space
    apply_w_h_cw_ch_r_c_m_s(root, -48, -22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, -21, 10, 20));
    // Layout out in a single row, width/4cols and negative height + margin/space
    apply_w_h_cw_ch_r_c_m_s(root, 48, -22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, -21, 10, 20));


    // And then all the same tests, but this time limited to 2 columns

    // cell size, 2 columns
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 0, 20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, 20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, -40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, -40, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 0, -40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, -40, 10, 20));

    // cell size, space/margin, 2 cols
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 1, 23, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23,  1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, 23, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, -43, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, -43, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 1, -43, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, -43, 10, 20));

    // width/height, 2x2 grid
    apply_w_h_cw_ch_r_c_m_s(root, 20, 40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10,  0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 0, 20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -20, 40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20,  0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, 20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -20, -40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, -40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, -40, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 20, -40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 0, -40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, -40, 10, 20));

    // width/height, 2x2 grid
    apply_w_h_cw_ch_r_c_m_s(root, 24, 44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 1, 23, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -24, 44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23,  1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, 23, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -24, -44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, -43, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, -43, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 24, -44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 1, -43, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, -43, 10, 20));


    cout << __PRETTY_FUNCTION__ << ": ok" << endl;

    root->destroy();
}

int main(int argc, char **argv)
{
    tst_layoutnode_properties();
    tst_layoutnode_horizontal_grid();

    return 0;
}