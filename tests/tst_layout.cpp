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

int main(int argc, char **argv)
{
    tst_layoutnode_properties();

    return 0;
}