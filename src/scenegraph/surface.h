#pragma once

namespace rengine
{

typedef void * NativeBufferHandle;

class Surface {
public:

    enum SurfaceFormat {
        AlphaFormatMask = 0x1000,
        RGBA_32 = 1 | AlphaFormatMask,
        RGBx_32 = 2,
    };

    /*!
       The size of the surface in pixels
     */
    virtual vec2 size() const = 0;

    /*!
        Returns the format of this surface.
     */
    virtual SurfaceFormat format() const = 0;

    /*!
        Returns true if the surface has alpha
     */
    bool hasAlpha() const { return (format() & AlphaFormatMask) != 0; }

    /*!
        Returns the texture id of the surface
     */
    virtual int textureId() const = 0;

    /*!
        Returns the native buffer handle for this surface.
     */
    virtual NativeBufferHandle nativeBufferHandle() const { return 0; }

};

}