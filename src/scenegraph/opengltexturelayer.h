#pragma once

namespace rengine {

class OpenGLTextureLayer : public Layer
{
    OpenGLTextureLayer()
        : m_id(0)
        , m_format(RGBA_32)
    {
    }

    ~OpenGLTextureLayer()
    {
        glDeleteTextures(1, &m_id);
    }

    /*!
       The size of the surface in pixels
     */
    vec2 size() const { return m_size; }

    /*!
        Returns the format of this surface.
     */
    Format format() const { return m_format; }

    /*!
        Returns the texture id of the surface
     */
    GLuint textureId() const { return m_id; }

    void upload(int width, int height, void *data)
    {
        if (m_id == 0) {
            glGenTextures(1, &m_id);
            glBindTexture(GL_TEXTURE_2D, m_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            glBindTexture(GL_TEXTURE_2D, m_id);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

private:
    GLuint m_id;
    Format m_format;
    vec2 m_size;

};

}
