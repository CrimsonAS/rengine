#pragma once

RENGINE_BEGIN_NAMESPACE



struct TransformNode_rotateAroundX {
    void operator()(double rotation, TransformNode *node) {
        node->setMatrix(mat4::rotateAroundX(rotation));
    }
};

struct TransformNode_rotateAroundY {
    void operator()(double rotation, TransformNode *node) {
        node->setMatrix(mat4::rotateAroundY(rotation));
    }
};

struct TransformNode_rotateAroundZ {
    void operator()(double rotation, TransformNode *node) {
        node->setMatrix(mat4::rotateAroundZ(rotation));
    }
};



struct OpacityNode_setOpacity {
    void operator()(double opacity, OpacityNode *node) {
        node->setOpacity(opacity);
    }
};



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



struct BlurNode_setRadius {
    void operator()(float radius, BlurNode *node) {
        node->setRadius(int(radius));
    }
};



struct ShadowNode_setRadius {
    void operator()(float radius, ShadowNode *node) {
        node->setRadius(int(radius));
    }
};

RENGINE_END_NAMESPACE