#include "common.h"
#include "shader.h"
#include <assert.h>
#include <iostream>
#include <string.h> // strncpy

static const char* vertexShaderSource =
        "uniform mat3 v_transform;"

        "attribute vec2 a_position;"

        "varying vec2 position;"
        "varying vec2 texcoord;"

        "void main() {"
            "texcoord = a_position;"
            "vec3 position = v_transform * vec3(a_position, 1.0);"
            "gl_Position = vec4(position.x, position.y, 0.5, position.z);"
        "}";

static const char* fragmentShaderBegin =
        "varying vec2 position;\n"
        "varying vec2 texcoord;\n";

static const char* fragmentShaderEnd =
        "\nvoid main() {"
        "   gl_FragColor = getcolor();\n"
        "}";

static const char* enumString(GLenum e)
{
    switch(e) {
    case GL_FRAGMENT_SHADER: return "fragment";
    case GL_VERTEX_SHADER: return "vertex shader";
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    default: return "unknown";
    }
}

// Returns 0 on failure
static GLuint compileShader(const std::string& src, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(shader, 1, &csrc, NULL);
    glCompileShader(shader);
    GLint compileSuccess = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE) {
        GLchar messages[1024];
        glGetShaderInfoLog(shader, sizeof(messages), 0, messages);
        std::cerr << "Error compiling " << enumString(type) << " shader:\n" << messages;
        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

// Returns 0 on failure
static GLuint createAndLinkProgram(GLuint vert, GLuint frag) {
    assert(vert != 0);
    assert(frag != 0);

    // Create and link program
    GLint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    // Report linking error if any
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLchar messages[1024];
        glGetProgramInfoLog(program, sizeof(messages), 0, messages);
        std::cerr << "Error linking shader program:\n" << messages;
        glDeleteProgram(program);
        return 0;
    }

    // Bind position attrib
    glBindAttribLocation(program, 0, "a_position");

    return program;
}

LUAEXPORT(void useShader(unsigned int i))
{
    glUseProgram(i);
}

LUAEXPORT(unsigned int addShader(const char *src))
{
    // Compile fragment shader
    std::string fragSource = fragmentShaderBegin + std::string(src) + fragmentShaderEnd;
    GLuint frag = compileShader(fragSource, GL_FRAGMENT_SHADER);
    if (frag == 0) return 0;

    // Compile vertex shader
    GLuint vert = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    if (vert == 0) {
        glDeleteShader(frag);
        return 0;
    }

    // Create shader program
    GLuint program = createAndLinkProgram(vert, frag);

    // If successful, use program and enable position attrib
    if (program != 0) {
        useShader(program);
        glEnableVertexAttribArray(0);
    }

    // No longer need the shaders
    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

LUAEXPORT(void setShaderParameter1(int loc, float x))
{
    glUniform1f(loc, x);
}

LUAEXPORT(void setShaderParameter2(int loc, float x, float y))
{
    glUniform2f(loc, x, y);
}

LUAEXPORT(void setShaderParameter3(int loc, float x, float y, float z))
{
    glUniform3f(loc, x, y, z);
}

LUAEXPORT(void setShaderParameter4(int loc, float x, float y, float z, float w))
{
    glUniform4f(loc, x, y, z, w);
}

LUAEXPORT(void setShaderParameter3x3(int loc, float* value))
{
    glUniformMatrix3fv(loc, 1, GL_TRUE, value);
}

LUAEXPORT(int getShaderParameterCount(int program)) {
    GLint nvars = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nvars);
    return nvars;
}

LUAEXPORT(bool getShaderParameterNameAndType(int program, int varindex, char* name, char* stype, int buflen))
{
    GLint size = 0;
    GLenum type = 0;
    glGetActiveUniform(program, varindex, buflen, 0, &size, &type, name);
    strncpy(stype, enumString(type), buflen);
    return true;
}
