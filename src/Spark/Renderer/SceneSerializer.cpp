#include "SceneSerializer.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "Entity.h"
#include "Components.h"
#include "Log.h"

namespace YAML {
    template<>
    struct convert<glm::vec2> {
        static Node encode(const glm::vec2& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }
        static bool decode(const Node& node, glm::vec2& rhs) {
            if (!node.IsSequence() || node.size() != 2) return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }
        static bool decode(const Node& node, glm::vec3& rhs) {
            if (!node.IsSequence() || node.size() != 3) return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }
        static bool decode(const Node& node, glm::vec4& rhs) {
            if (!node.IsSequence() || node.size() != 4) return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
    : m_Scene(scene) {}

static void SerializeEntity(YAML::Emitter& out, Entity entity) {
    out << YAML::BeginMap; // Entity
    if (entity.HasComponent<IDComponent>())
        out << YAML::Key << "Entity" << YAML::Value << (uint64_t)entity.GetComponent<IDComponent>().ID;
    else
        out << YAML::Key << "Entity" << YAML::Value << (uint64_t)Spark::UUID(); // Assign new UUID if missing

    if (entity.HasComponent<TagComponent>()) {
        out << YAML::Key << "TagComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().Tag;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<TransformComponent>()) {
        out << YAML::Key << "TransformComponent";
        out << YAML::BeginMap;
        auto& tc = entity.GetComponent<TransformComponent>();
        out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
        out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
        out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<SpriteRendererComponent>()) {
        out << YAML::Key << "SpriteRendererComponent";
        out << YAML::BeginMap;
        auto& src = entity.GetComponent<SpriteRendererComponent>();
        out << YAML::Key << "Color" << YAML::Value << src.Color;
        out << YAML::Key << "TextureHandle" << YAML::Value << (uint64_t)src.TextureHandle;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<CircleRendererComponent>()) {
        out << YAML::Key << "CircleRendererComponent";
        out << YAML::BeginMap;
        auto& crc = entity.GetComponent<CircleRendererComponent>();
        out << YAML::Key << "Color" << YAML::Value << crc.Color;
        out << YAML::Key << "Thickness" << YAML::Value << crc.Thickness;
        out << YAML::Key << "Fade" << YAML::Value << crc.Fade;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<Rigidbody2DComponent>()) {
        out << YAML::Key << "Rigidbody2DComponent";
        out << YAML::BeginMap;
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        out << YAML::Key << "Type" << YAML::Value << (int)rb2d.Type;
        out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<BoxCollider2DComponent>()) {
        out << YAML::Key << "BoxCollider2DComponent";
        out << YAML::BeginMap;
        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
        out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
        out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
        out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
        out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
        out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
        out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2d.RestitutionThreshold;
        out << YAML::Key << "IsSensor" << YAML::Value << bc2d.IsSensor;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
        out << YAML::Key << "CircleCollider2DComponent";
        out << YAML::BeginMap;
        auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
        out << YAML::Key << "Offset" << YAML::Value << cc2d.Offset;
        out << YAML::Key << "Radius" << YAML::Value << cc2d.Radius;
        out << YAML::Key << "Density" << YAML::Value << cc2d.Density;
        out << YAML::Key << "Friction" << YAML::Value << cc2d.Friction;
        out << YAML::Key << "Restitution" << YAML::Value << cc2d.Restitution;
        out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2d.RestitutionThreshold;
        out << YAML::Key << "IsSensor" << YAML::Value << cc2d.IsSensor;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<LuaScriptComponent>()) {
        out << YAML::Key << "LuaScriptComponent";
        out << YAML::BeginMap;
        auto& script = entity.GetComponent<LuaScriptComponent>();
        out << YAML::Key << "Path" << YAML::Value << script.Path;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<AudioSourceComponent>()) {
        out << YAML::Key << "AudioSourceComponent";
        out << YAML::BeginMap;
        auto& audio = entity.GetComponent<AudioSourceComponent>();
        out << YAML::Key << "Path" << YAML::Value << audio.Path;
        out << YAML::Key << "Volume" << YAML::Value << audio.Volume;
        out << YAML::Key << "Pitch" << YAML::Value << audio.Pitch;
        out << YAML::Key << "Loop" << YAML::Value << audio.Loop;
        out << YAML::Key << "PlayOnStart" << YAML::Value << audio.PlayOnStart;
        out << YAML::Key << "Spatial" << YAML::Value << audio.Spatial;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<AudioListenerComponent>()) {
        out << YAML::Key << "AudioListenerComponent";
        out << YAML::BeginMap;
        auto& listener = entity.GetComponent<AudioListenerComponent>();
        out << YAML::Key << "Active" << YAML::Value << listener.Active;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<CameraComponent>()) {
        out << YAML::Key << "CameraComponent";
        out << YAML::BeginMap;
        auto& cc = entity.GetComponent<CameraComponent>();
        auto& camera = cc.Camera;

        out << YAML::Key << "Camera" << YAML::BeginMap;
        out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
        out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
        out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
        out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
        out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
        out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
        out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
        out << YAML::EndMap;

        out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
        out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.FixedAspectRatio;
        out << YAML::EndMap;
    }

    out << YAML::EndMap; // Entity
}

void SceneSerializer::Serialize(const std::string& filepath) {
    SP_INFO("Serializing scene to: " + filepath);
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Untitled";
    out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

    for (auto entityID : m_Scene->m_Registry.storage<entt::entity>()) {
        Entity entity(entityID, m_Scene.get());
        if (!entity) continue;
        SerializeEntity(out, entity);
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;

    std::ofstream fout(filepath);
    fout << out.c_str();
    SP_INFO("Scene serialized successfully.");
}

bool SceneSerializer::Deserialize(const std::string& filepath) {
    SP_INFO("Deserializing scene from: " + filepath);
    std::ifstream stream(filepath);
    if (!stream) {
        SP_ERROR("Could not open scene file: " + filepath);
        return false;
    }

    std::stringstream strStream;
    strStream << stream.rdbuf();

    YAML::Node data;
    try {
        data = YAML::Load(strStream.str());
    } catch (const YAML::Exception& e) {
        SP_ERROR("Failed to load YAML: " + std::string(e.what()));
        return false;
    }

    if (!data["Scene"]) {
        SP_ERROR("Invalid scene file format.");
        return false;
    }

    auto entities = data["Entities"];
    if (entities) {
        for (auto entity : entities) {
            uint64_t uuid = entity["Entity"].as<uint64_t>();

            std::string name;
            auto tagComponent = entity["TagComponent"];
            if (tagComponent) name = tagComponent["Tag"].as<std::string>();

            Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

            auto transformComponent = entity["TransformComponent"];
            if (transformComponent) {
                auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                tc.Scale = transformComponent["Scale"].as<glm::vec3>();
            }

            auto spriteRendererComponent = entity["SpriteRendererComponent"];
            if (spriteRendererComponent) {
                auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                if (spriteRendererComponent["TextureHandle"])
                    src.TextureHandle = spriteRendererComponent["TextureHandle"].as<uint64_t>();
            }

            auto circleRendererComponent = entity["CircleRendererComponent"];
            if (circleRendererComponent) {
                auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
                crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
                crc.Thickness = circleRendererComponent["Thickness"].as<float>();
                crc.Fade = circleRendererComponent["Fade"].as<float>();
            }

            auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
            if (rigidbody2DComponent) {
                auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
                rb2d.Type = (Rigidbody2DComponent::BodyType)rigidbody2DComponent["Type"].as<int>();
                rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
            }

            auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
            if (boxCollider2DComponent) {
                auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
                bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                bc2d.Density = boxCollider2DComponent["Density"].as<float>();
                bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
                bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
                bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
                if (boxCollider2DComponent["IsSensor"])
                    bc2d.IsSensor = boxCollider2DComponent["IsSensor"].as<bool>();
            }

            auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
            if (circleCollider2DComponent) {
                auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
                cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
                cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
                cc2d.Density = circleCollider2DComponent["Density"].as<float>();
                cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
                cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
                cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
                if (circleCollider2DComponent["IsSensor"])
                    cc2d.IsSensor = circleCollider2DComponent["IsSensor"].as<bool>();
            }

            auto luaScriptComponent = entity["LuaScriptComponent"];
            if (luaScriptComponent) {
                auto& script = deserializedEntity.AddComponent<LuaScriptComponent>();
                script.Path = luaScriptComponent["Path"].as<std::string>();
            }

            auto audioSourceComponent = entity["AudioSourceComponent"];
            if (audioSourceComponent) {
                auto& audio = deserializedEntity.AddComponent<AudioSourceComponent>();
                audio.Path = audioSourceComponent["Path"].as<std::string>();
                audio.Volume = audioSourceComponent["Volume"].as<float>();
                audio.Pitch = audioSourceComponent["Pitch"].as<float>();
                audio.Loop = audioSourceComponent["Loop"].as<bool>();
                audio.PlayOnStart = audioSourceComponent["PlayOnStart"].as<bool>();
                audio.Spatial = audioSourceComponent["Spatial"].as<bool>();
            }

            auto audioListenerComponent = entity["AudioListenerComponent"];
            if (audioListenerComponent) {
                auto& listener = deserializedEntity.AddComponent<AudioListenerComponent>();
                listener.Active = audioListenerComponent["Active"].as<bool>();
            }

            auto cameraComponent = entity["CameraComponent"];
            if (cameraComponent) {
                auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                auto cameraNode = cameraComponent["Camera"];
                if (cameraNode) {
                    cc.Camera.SetProjectionType((Spark::SceneCamera::ProjectionType)cameraNode["ProjectionType"].as<int>());
                    cc.Camera.SetPerspectiveVerticalFOV(cameraNode["PerspectiveFOV"].as<float>());
                    cc.Camera.SetPerspectiveNearClip(cameraNode["PerspectiveNear"].as<float>());
                    cc.Camera.SetPerspectiveFarClip(cameraNode["PerspectiveFar"].as<float>());
                    cc.Camera.SetOrthographicSize(cameraNode["OrthographicSize"].as<float>());
                    cc.Camera.SetOrthographicNearClip(cameraNode["OrthographicNear"].as<float>());
                    cc.Camera.SetOrthographicFarClip(cameraNode["OrthographicFar"].as<float>());
                }
                cc.Primary = cameraComponent["Primary"].as<bool>();
                cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
            }
        }
    }
    
    SP_INFO("Scene deserialized successfully.");
    return true;
}
