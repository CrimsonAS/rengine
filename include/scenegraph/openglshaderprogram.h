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

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
*/

#pragma once

#include <iostream>
#include <vector>

RENGINE_BEGIN_NAMESPACE

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

RENGINE_END_NAMESPACE
