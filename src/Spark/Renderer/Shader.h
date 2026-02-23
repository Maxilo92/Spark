#pragma once
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void UploadUniformInt(const std::string& name, int value);
    void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
    void UploadUniformFloat4(const std::string& name, const glm::vec4& value);
    void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

    uint32_t m_RendererID;
};
