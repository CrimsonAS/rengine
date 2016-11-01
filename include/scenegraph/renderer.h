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

RENGINE_BEGIN_NAMESPACE

class Renderer
{
public:
    Renderer()
        : m_sceneRoot(0)
        , m_surface(0)
        , m_fillColor(0, 0, 0, 1)
    {
    }

    virtual ~Renderer() {}

    /*!
        Creates a texture from image data which is compatible with this
        renderer. The image data is 32-bit RGBA or RGBx, tightly packed.
     */
    virtual Texture *createTextureFromImageData(vec2 size, Texture::Format format, void *data) = 0;

    Node *sceneRoot() const { return m_sceneRoot; }
    void setSceneRoot(Node *root) { m_sceneRoot = root; }

    Surface *targetSurface() const { return m_surface; }
    void setTargetSurface(Surface *surface) { m_surface = surface; }

    /*!
        The application should call this before the very first render pass.

        Once this function is called, the scene graph can be built and
        put to the renderer.
     */
    virtual void initialize() = 0;

    /*!
        Renders the scene graph specified using setSceneRoot().

        Returns true if successful; false if rendering failed...
     */
    virtual bool render() = 0;

    /*!
        Read back pixels into \a bytes.

        The pixels will be 32 bit RGBA and tightly packed into \a width and \a
        height.
     */
    virtual bool readPixels(int x, int y, int width, int height, unsigned *bytes) = 0;

    /*!
        Called after the frame has been swapped. The renderer can use this
        to perform post-frame cleanup, for instance...
     */
    virtual void frameSwapped() { }

    void setFillColor(vec4 c) { m_fillColor = c; }
    vec4 fillColor() const { return m_fillColor; }

#if 0
    Texture *createTextureFromSubtree(Node *node, rect2d sourceRect);
    Texture *createTextureWithBlurFromTexture(Texture *texture, int kernelRadius);
    Texture *createTextureWithShadowFromTexture(Texture *texture, int kernelRadius, vec2 offset,  color);
    Texture *createTextureWithColorFilterFromTexture(Texture *texture, mat4 colorMatrix);
protected:

    /*!
        Creates a render target inside the renderer, like an FBO in OpenGL and
        makes that render target active.

        The render target will have \a size dimensions.

        This function should be used by createTextureFromSubtree and the likes
        to create temporary render targets.

        This function shall never be called during rendering.
     */
    virtual void openRenderTarget(vec2 size) = 0;

    /*!
        Closes the current render target and returns a texture representing it.

        The ownership of the returned texture is transferred to the caller.
     */
    virtual Texture *closeRenderTarget() = 0;
#endif

private:
    Node *m_sceneRoot;
    Surface *m_surface;
    vec4 m_fillColor;
};

#if 0
inline Texture *Renderer::createTextureFromSubtree(Node *node, rect2d sourceRect)
{
    assert(node);
    assert(!node->parent());

    rect2d rect(floor(sourceRect.tl), ceil(sourceRect.br));

    openRenderTarget(rect.size());

    TransformNode xnode;
    xnode.setMatrix(mat4::translate2D(-rect.position()));
    xnode << node;

    Node *oldRoot = sceneRoot();
    setSceneRoot(&xnode);
    render();
    setSceneRoot(oldRoot);

    xnode.remove(node);

    return closeRenderTarget();
}

inline Texture *createTextureWithBlurFromTexture(Texture *texture, int kernelRadius)
{
    assert(kernelRadius > 0);
    assert(texture);

    BlurNode blurNode;
    blurNode.setRadius(kernelRadius);

    TextureNode textureNode;
    textureNode.setTexture(texture);
    textureNode.setGeometry(rect2d::fromPosSize(vec2 (kernelRadius, kernelRadius), texture->size()))

    blurNode.append(&textureNode);

    return createTextureFromSubtree(&blurNode, rect2d(vec2(0, 0), texture->size() + kernelRadius * 2));
}

inline Texture *createTextureWithShadowFromTexture(Texture *texture, int kernelRadius, vec2 offset,  color)
{
    assert(kernelRadius > 0);
    assert(texture);

    ShadowNode shadowNode;
    shadowNode.setRadius(kernelRadius);
    shadowNode.setOffset(offset);
    shadowNode.setColor(color);

    TextureNode textureNode;
    textureNode.setTexture(texture);
    textureNode.setGeometry(rect2d::fromPosSize(vec2(kernelRadius, kernelRadius), texture->size()))

    blurNode.append(&textureNode);

    return createTextureFromSubtree(&blurNode, rect2d(vec2(0, 0), texture->size() + kernelRadius * 2));
}
#endif


RENGINE_END_NAMESPACE