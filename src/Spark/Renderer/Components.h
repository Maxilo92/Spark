#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include <sol/sol.hpp>
#include "SubTexture2D.h"
#include "UUID.h"
#include <vector>

struct IDComponent {
    Spark::UUID ID;

    IDComponent() = default;
    IDComponent(const IDComponent&) = default;
    IDComponent(Spark::UUID id) : ID(id) {}
};

struct LuaScriptComponent {
    std::string Path;
    sol::environment Environment;
    bool Initialized = false;

    LuaScriptComponent() = default;
    LuaScriptComponent(const std::string& path) : Path(path) {}
};

struct TagComponent {
    std::string Tag;
    TagComponent() = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};

struct TransformComponent {
    glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

    TransformComponent() = default;
    TransformComponent(const glm::vec3& translation) : Translation(translation) {}
    
    glm::mat4 GetTransform() const {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), { 1, 0, 0 })
                           * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), { 0, 1, 0 })
                           * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), { 0, 0, 1 });

        return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
    }
};

struct SpriteRendererComponent {
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    Spark::AssetHandle TextureHandle = 0;
    std::shared_ptr<Spark::SubTexture2D> SubTexture = nullptr;

    SpriteRendererComponent() = default;
    SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
};

struct CircleRendererComponent {
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    float Thickness = 1.0f;
    float Fade = 0.005f;

    CircleRendererComponent() = default;
    CircleRendererComponent(const glm::vec4& color) : Color(color) {}
};

struct SpriteAnimationComponent {
    std::vector<std::shared_ptr<Spark::SubTexture2D>> Frames;
    float FrameTime = 0.1f; // Sekunden pro Frame
    int CurrentFrame = 0;
    float Timer = 0.0f;
    bool Loop = true;
    bool Playing = true;

    SpriteAnimationComponent() = default;
    SpriteAnimationComponent(const std::vector<std::shared_ptr<Spark::SubTexture2D>>& frames) : Frames(frames) {}
};

struct Rigidbody2DComponent {
    enum class BodyType { Static = 0, Kinematic, Dynamic };
    BodyType Type = BodyType::Static;
    bool FixedRotation = false;

    // Speicher für den Box2D Körper zur Laufzeit
    void* RuntimeBody = nullptr;

    Rigidbody2DComponent() = default;
    Rigidbody2DComponent(BodyType type) : Type(type) {}
};

struct BoxCollider2DComponent {
    glm::vec2 Offset = { 0.0f, 0.0f };
    glm::vec2 Size = { 0.5f, 0.5f };

    float Density = 1.0f;
    float Friction = 0.5f;
    float Restitution = 0.0f;
    float RestitutionThreshold = 0.5f;

    // Speicher für die Box2D Fixture zur Laufzeit
    void* RuntimeFixture = nullptr;

    BoxCollider2DComponent() = default;
    BoxCollider2DComponent(const glm::vec2& size) : Size(size) {}
    BoxCollider2DComponent(const glm::vec2& offset, const glm::vec2& size) : Offset(offset), Size(size) {}
};

struct CircleCollider2DComponent {
    glm::vec2 Offset = { 0.0f, 0.0f };
    float Radius = 0.5f;

    float Density = 1.0f;
    float Friction = 0.5f;
    float Restitution = 0.0f;
    float RestitutionThreshold = 0.5f;

    // Speicher für die Box2D Fixture zur Laufzeit
    void* RuntimeFixture = nullptr;

    CircleCollider2DComponent() = default;
};

struct AudioSourceComponent {
    std::string Path;
    float Volume = 1.0f;
    float Pitch = 1.0f;
    bool Loop = false;
    bool PlayOnStart = true;
    bool Spatial = true;

    // Runtime
    void* RuntimeSound = nullptr; // ma_sound*

    AudioSourceComponent() = default;
    AudioSourceComponent(const std::string& path) : Path(path) {}
};

struct AudioListenerComponent {
    bool Active = true;

    AudioListenerComponent() = default;
};
