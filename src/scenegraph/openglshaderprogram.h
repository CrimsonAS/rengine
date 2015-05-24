#pragma once

#include <iostream>

namespace rengine
{

class OpenGLShaderProgram
{
public:
    OpenGLShaderProgram()
    : m_id(0)
    {
    }

    GLuint id() const { return m_id; }

    GLint resolve(const char *name) {
        GLint id = glGetUniformLocation(m_id, name);
        if (id < 0) {
            std::cout << "error: failed to resolve uniform: '" << name << "'.." << std::endl;
            assert(false);
        }
        return id;
    }

    GLuint createShader(const char *sh, GLenum type)
    {
        GLuint id = glCreateShader(type);
        int len = strlen(sh);
        glShaderSource(id, 1, &sh, &len);
        glCompileShader(id);
        int param = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &param);
        if (param == GL_FALSE) {
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &param);
            char *str = (char *) malloc(param + 1);
            int l = 0;
            glGetShaderInfoLog(id, param, &l, str);
            assert(l < param);
            str[l] = '\0';
            std::cout << "error: Failed to compile shader: " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl
                 << sh << std::endl
                 << "error: " << str << std::endl;
            free(str);
            assert(false);
        }
        return id;
    }

    void initialize(const char *vsh, const char *fsh, const std::vector<const char *> &attrs)
    {
        assert(m_id == 0);

        GLuint vid = createShader(vsh, GL_VERTEX_SHADER);
        GLuint fid = createShader(fsh, GL_FRAGMENT_SHADER);
        assert(vid);
        assert(fid);

        m_id = glCreateProgram();
        glAttachShader(m_id, vid);
        glAttachShader(m_id, fid);

        for (int i=0; i<attrs.size(); ++i)
            glBindAttribLocation(m_id, i, attrs.at(i));

        glLinkProgram(m_id);

        int param = 0;
        glGetProgramiv(m_id, GL_LINK_STATUS, &param);
        if (param == GL_FALSE) {
            glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &param);
            char *str = (char *) malloc(param + 1);
            int l = 0;
            glGetProgramInfoLog(m_id, param, &l, str);
            assert(l < param);
            str[l] = '\0';
            std::cout << "error: Failed to link shader program:" << std::endl
                 << "Vertex Shader:" << std::endl << vsh << std::endl
                 << "FragmentShader:" << std::endl << fsh << std::endl
                 << "error: " << str << std::endl;
            free(str);
            assert(false);
        }

        assert(glGetError() == GL_NO_ERROR);
    }

private:
    GLuint m_id;
};

}