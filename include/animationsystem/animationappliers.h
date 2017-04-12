/*
 * Copyright (c) 2017 Crimson AS <info@crimson.no>
 * Author: Gunnar Sletta <gunnar@crimson.no>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

RENGINE_BEGIN_NAMESPACE


typedef Animation<TransformNode, float, &TransformNode::setMatrix_rotateAroundX> Animation_TransformNode_rotateAroundX;
typedef Animation<TransformNode, float, &TransformNode::setMatrix_rotateAroundY> Animation_TransformNode_rotateAroundY;
typedef Animation<TransformNode, float, &TransformNode::setMatrix_rotateAroundZ> Animation_TransformNode_rotateAroundZ;
typedef Animation<TransformNode, float, &TransformNode::setMatrix_rotate2D> Animation_TransformNode_rotate2D;

typedef Animation<OpacityNode, float, &OpacityNode::setOpacity> Animation_OpacityNode_opacity;

typedef Animation<BlurNode, unsigned int, &BlurNode::setRadius> Animation_BlurNode_radius;

typedef Animation<ShadowNode, unsigned int, &ShadowNode::setRadius> Animation_ShadowNode_radius;


struct ColorFilterNode_saturation {
    void operator()(double saturation, ColorFilterNode *node) {
        node->setColorMatrix(ColorMatrix::saturation(saturation));
    }
};

struct ColorFilterNode_brightness {
    void operator()(double brightness, ColorFilterNode *node) {
        node->setColorMatrix(ColorMatrix::brightness(brightness));
    }
};

struct ColorFilterNode_hue{
    void operator()(double hue, ColorFilterNode *node) {
        node->setColorMatrix(ColorMatrix::hue(hue));
    }
};

struct ColorFilterNode_contrast {
    void operator()(double contrast, ColorFilterNode *node) {
        node->setColorMatrix(ColorMatrix::contrast(contrast));
    }
};

struct ColorFilterNode_invert {
    void operator()(double invert, ColorFilterNode *node) {
        node->setColorMatrix(ColorMatrix::invert(invert));
    }
};

struct ColorFilterNode_sepia {
    void operator()(double sepia, ColorFilterNode *node) {
        node->setColorMatrix(ColorMatrix::sepia(sepia));
    }
};


RENGINE_END_NAMESPACE
