#include "Shader.h"
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
    // Vertex Shader erstellen
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vSource = vertexSrc.c_str();
    glShaderSource(vertexShader, 1, &vSource, nullptr);
    glCompileShader(vertexShader);

    // Fehlerprüfung Vertex Shader
    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        glDeleteShader(vertexShader);
        std::cerr << "Vertex Shader Compilation Failed: " << infoLog.data() << std::endl;
        return;
    }

    // Fragment Shader erstellen
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSource = fragmentSrc.c_str();
    glShaderSource(fragmentShader, 1, &fSource, nullptr);
    glCompileShader(fragmentShader);

    // Fehlerprüfung Fragment Shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        std::cerr << "Fragment Shader Compilation Failed: " << infoLog.data() << std::endl;
        return;
    }

    // Shader Programm erstellen und verlinken
    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertexShader);
    glAttachShader(m_RendererID, fragmentShader);
    glLinkProgram(m_RendererID);

    // Fehlerprüfung Linker
    GLint isLinked = 0;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);
        glDeleteProgram(m_RendererID);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        std::cerr << "Shader Linking Failed: " << infoLog.data() << std::endl;
        return;
    }

    // Shader aufräumen
    glDetachShader(m_RendererID, vertexShader);
    glDetachShader(m_RendererID, fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const {
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::UploadUniformInt(const std::string& name, int value) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform1i(location, value);
}

void Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
}
