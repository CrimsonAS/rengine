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

    // cellsize, one row
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, 0, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, 0, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, -20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, -20, 10, 20));

    // cellsize, margin/spacing
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, 1, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, 1, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, -21, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, -21, 10, 20));

    // width/height, 1x4 grid
    apply_w_h_cw_ch_r_c_m_s(root, 40, 20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, 0, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -40, 20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, 0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, 0, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, 0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, 0, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -40, -20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-30, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-40, -20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 40, -20, 0, 0, 1, 4, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(20, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(30, -20, 10, 20));

    // width/height, 1x4 grid, space/margin
    apply_w_h_cw_ch_r_c_m_s(root, 48, 22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(25, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(37, 1, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -48, 22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, 1, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, 1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, 1, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -48, -22, 0, 0, 1, 4, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-35, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-47, -21, 10, 20));
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


void tst_layoutnode_vertical_grid()
{
    LayoutNode *root = LayoutNode::create();
    root->setActivationMode(LayoutNode::Explicit);
    root->setLayoutType(LayoutEngine::Grid_Vertical);

    RectangleNode *r[4];
    for (int i=0; i<4; ++i) {
        r[i] = RectangleNode::create();
        root->append(r[i]);
    }

    // cellsize, one column
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(0,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(0, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(0, 40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(0, 60, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, 40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-10, 60, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, -60, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-10, -80, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(0, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(0, -60, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(0, -80, 10, 20));

    // cellsize, one column, margin/spacing
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(1,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(1, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(1, 45, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(1, 67, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, 45, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-11, 67, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, -65, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-11, -87, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 0, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(1, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(1, -65, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(1, -87, 10, 20));

    // width/height, 4x1 grid
    apply_w_h_cw_ch_r_c_m_s(root, 10, 80, 0, 0, 4, 1, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(0,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(0, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(0, 40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(0, 60, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -10, 80, 0, 0, 4, 1, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, 40, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-10, 60, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -10, -80, 0, 0, 4, 1, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-10, -60, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-10, -80, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 10, -80, 0, 0, 4, 1, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(0, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(0, -60, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(0, -80, 10, 20));

    // width/height, 4x1 grid, space/margin
    apply_w_h_cw_ch_r_c_m_s(root, 12, 88, 0, 0, 4, 1, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(1, 1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(1, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(1, 45, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(1, 67, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -12, 88, 0, 0, 4, 1, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, 45, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-11, 67, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -12, -88, 0, 0, 4, 1, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-11, -65, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-11, -87, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 12, -88, 0, 0, 4, 1, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(1, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(1, -65, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(1, -87, 10, 20));

    // And then all the same tests, but this time limited to 2 rows

    // cell size, 2 rows
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 2, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 0, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(10,  0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 2, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-20,  0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 2, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, -40, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 2, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 0, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, -40, 10, 20));

    // cell size, space/margin, 2 rows
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, 20, 2, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 1, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(13,  1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, 20, 2, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-23,  1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, -10, -20, 2, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, -43, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 0, 0, 10, -20, 2, 0, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 1, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, -43, 10, 20));

    // width/height, 2x2 grid
    apply_w_h_cw_ch_r_c_m_s(root, 20, 40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 0, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(10,  0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -20, 40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  0, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, 20, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-20,  0, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, 20, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -20, -40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-10, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-20, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-20, -40, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 20, -40, 0, 0, 2, 2, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -20, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 0, -40, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(10, -20, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(10, -40, 10, 20));

    // width/height, 2x2 grid
    apply_w_h_cw_ch_r_c_m_s(root, 24, 44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 1, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(13,  1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -24, 44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11,  1, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, 23, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-23,  1, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, 23, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, -24, -44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-11, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-11, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-23, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-23, -43, 10, 20));
    apply_w_h_cw_ch_r_c_m_s(root, 24, -44, 0, 0, 2, 2, 1, 2);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 1, -21, 10, 20));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 1, -43, 10, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(13, -21, 10, 20));
    check_equal(r[3]->geometry(), rect2d::fromXywh(13, -43, 10, 20));

    cout << __PRETTY_FUNCTION__ << ": ok" << endl;

    root->destroy();
}

static void apply_w_h_r_c_m_s(LayoutNode *node, float width, float height, int rowCount, int columnCount, float margin, float spacing)
{
    node->setWidth(width);
    node->setHeight(height);
    node->setRowCount(rowCount);
    node->setColumnCount(columnCount);
    node->setSpacing(spacing);
    node->setMargin(margin);
    node->updateLayout();
}


static void tst_layoutnode_horizontal_flow()
{
    LayoutNode *root = LayoutNode::create();
    root->setActivationMode(LayoutNode::Explicit);
    root->setLayoutType(LayoutEngine::Flow_Horizontal);

    float inf = std::numeric_limits<float>::infinity();

    RectangleNode *r[4]; // sizes: 10, 20, 40, 80
    for (int i=0; i<4; ++i) {
        float s = std::pow(2, (float)i) * 10;
        r[i] = RectangleNode::create(rect2d(0, 0, s, s));
        root->append(r[i]);
    }

    // Horizontal, positive width/height
    apply_w_h_r_c_m_s(root, 0, 0, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, 0, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, 0, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(30, 0, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(70, 0, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 0, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, 2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34, 2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(75, 2, 80, 80));
    apply_w_h_r_c_m_s(root, 78, 0, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34,  2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh( 2, 43, 80, 80));
    apply_w_h_r_c_m_s(root, 77.99, 0, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 2, 23, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh( 2, 64, 80, 80));
    apply_w_h_r_c_m_s(root, 4, 0, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh(2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(2, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(2, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(2, 75, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 0, 0, 2, 2, 1); // will not fit the number of items, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 2, 23, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(43, 23, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 0, 0, 1, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh(2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(2, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(2, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(2, 75, 80, 80));

    // Horizontal, negative layout direction..
    apply_w_h_r_c_m_s(root, -inf, 0, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( -10, 0, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -30, 0, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -70, 0, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-150, 0, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 0, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, 2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33, 2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -74, 2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-155, 2, 80, 80));
    apply_w_h_r_c_m_s(root, -78, 0, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-33,  2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-74,  2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, 43, 80, 80));
    apply_w_h_r_c_m_s(root, -77.99, 0, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-33,  2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42, 23, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, 64, 80, 80));
    apply_w_h_r_c_m_s(root, -4, 0, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, 75, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 0, 0, 2, 2, 1); // will not fit rows, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,   2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-33,   2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42,  23, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-123, 23, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 0, 0, 1, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, 75, 80, 80));

    // Horizontal, positive width, negative height
    apply_w_h_r_c_m_s(root, 0, -inf, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 0, -10, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(10, -20, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(30, -40, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(70, -80, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -inf, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(75, -82, 80, 80));
    apply_w_h_r_c_m_s(root, 78, -inf, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34,  -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh( 2, -123, 80, 80));
    apply_w_h_r_c_m_s(root, 77.99, -inf, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 2, -63, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh( 2, -144, 80, 80));
    apply_w_h_r_c_m_s(root, 4, -inf, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh(2,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(2,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(2,  -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(2, -155, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -inf, 0, 2, 2, 1); // will not fit the number of items, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13,  -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 2,  -63, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(43, -103, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -inf, 0, 1, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh(2,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(2,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(2,  -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(2, -155, 80, 80));

    // Horizontal, negative directions...
    apply_w_h_r_c_m_s(root, -inf, -inf, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh( -10, -10, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -30, -20, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -70, -40, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-150, -80, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -inf, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -74, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-155, -82, 80, 80));
    apply_w_h_r_c_m_s(root, -78, -inf, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-33,  -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-74,  -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, -123, 80, 80));
    apply_w_h_r_c_m_s(root, -77.99, -inf, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-33,  -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42,  -63, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, -144, 80, 80));
    apply_w_h_r_c_m_s(root, -4, -inf, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42,  -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, -155, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -inf, 0, 2, 2, 1); // will not fit rows, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33,  -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -42,  -63, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-123, -103 , 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -inf, 0, 1, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42,  -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, -155, 80, 80));


    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}

static void tst_layoutnode_vertical_flow()
{
    LayoutNode *root = LayoutNode::create();
    root->setActivationMode(LayoutNode::Explicit);
    root->setLayoutType(LayoutEngine::Flow_Vertical);

    float inf = std::numeric_limits<float>::infinity();

    RectangleNode *r[4]; // sizes: 10, 20, 40, 80
    for (int i=0; i<4; ++i) {
        float s = std::pow(2, (float)i) * 10;
        r[i] = RectangleNode::create(rect2d(0, 0, s, s));
        root->append(r[i]);
    }

    // Vertical, positive width/height
    apply_w_h_r_c_m_s(root, 0, 0, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(0,  0, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(0, 10, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(0, 30, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(0, 70, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 0, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh(2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(2, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(2, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(2, 75, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 78, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 2, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 2, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(43,  2, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 77.99, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 2, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(23,  2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(64,  2, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 4, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, 2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34, 2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(75, 2, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 0, 2, 0, 2, 1); // will not fit the number of items, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 2, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(23,  2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(23, 43, 80, 80));
    apply_w_h_r_c_m_s(root, 0, 0, 1, 0, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, 2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, 2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34, 2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(75, 2, 80, 80));

    // Vertical, negative layout direction..
    apply_w_h_r_c_m_s(root, 0, -inf, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(0,  -10, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(0,  -30, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(0,  -70, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(0, -150, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -inf, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh(2,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(2,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(2,  -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(2, -155, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -78, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 2, -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( 2, -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(43, -82, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -77.99, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 2, -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(23, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(64, -82, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -4, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(75, -82, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -inf, 2, 0, 2, 1); // will not fit rows, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12,  10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( 2, -33,  20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(23, -42,  40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(23, -123, 80, 80));
    apply_w_h_r_c_m_s(root, 0, -inf, 1, 0, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh( 2, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(13, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(34, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(75, -82, 80, 80));

    // Vertical, positive layout direction, negative other direction
    apply_w_h_r_c_m_s(root, -inf, 0, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  0, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20, 10, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-40, 30, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-80, 70, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 0, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, 75, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 78, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -22, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -42, 34, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-123,  2, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 77.99, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-63,  2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-144, 2,  80, 80));
    apply_w_h_r_c_m_s(root, -inf, 4, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, 2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33, 2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -74, 2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-155, 2, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 0, 2, 0, 2, 1); // will not fit the number of items, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12,  2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -22, 13, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -63,  2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-103, 43, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, 0, 1, 0, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, 2, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33, 2, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -74, 2, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-155, 2, 80, 80));

    // Vertical, negative directions...
    apply_w_h_r_c_m_s(root, -inf, -inf, 0, 0, 0, 0);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-10,  -10, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-20,  -30, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-40,  -70, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-80, -150, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -inf, 0, 0, 2, 1);
    check_equal(r[0]->geometry(), rect2d::fromXywh(-12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh(-22,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh(-42,  -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-82, -155, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -78, 0, 0, 2, 1); // r[2] ends on 74, +4 (2+2) margin
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -22, -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -42, -74, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-123, -82, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -77.99, 0, 0, 2, 1); // will not fit the, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -22, -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -63, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-144, -82, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -4, 0, 0, 2, 1); // too narrow width, but we always have one per row
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -74, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-155, -82, 80, 80));
    apply_w_h_r_c_m_s(root, -inf, -inf, 2, 0, 2, 1); // will not fit rows, so r[2] wraps
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12,  -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -22,  -33, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -63,  -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-103, -123,  80, 80));
    apply_w_h_r_c_m_s(root, -inf, -inf, 1, 0, 2, 1); // single-column, wrap all
    check_equal(r[0]->geometry(), rect2d::fromXywh( -12, -12, 10, 10));
    check_equal(r[1]->geometry(), rect2d::fromXywh( -33, -22, 20, 20));
    check_equal(r[2]->geometry(), rect2d::fromXywh( -74, -42, 40, 40));
    check_equal(r[3]->geometry(), rect2d::fromXywh(-155, -82, 80, 80));


    cout << __PRETTY_FUNCTION__ << ": ok" << endl;
}


int main(int argc, char **argv)
{
    tst_layoutnode_properties();
    tst_layoutnode_horizontal_grid();
    tst_layoutnode_vertical_grid();
    tst_layoutnode_horizontal_flow();
    tst_layoutnode_vertical_flow();

    return 0;
}