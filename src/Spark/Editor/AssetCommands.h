#pragma once
#include "Command.h"
#include "Entity.h"
#include "Components.h"
#include "Asset.h"

namespace Spark {

    class SpriteChangeCommand : public Command {
    public:
        SpriteChangeCommand(Entity entity, AssetHandle oldHandle, AssetHandle newHandle)
            : m_Entity(entity), m_OldHandle(oldHandle), m_NewHandle(newHandle) {}

        void Execute() override {
            m_Entity.GetComponent<SpriteRendererComponent>().TextureHandle = m_NewHandle;
        }

        void Undo() override {
            m_Entity.GetComponent<SpriteRendererComponent>().TextureHandle = m_OldHandle;
        }

        std::string GetName() const override { return "Change Sprite Texture"; }

    private:
        Entity m_Entity;
        AssetHandle m_OldHandle, m_NewHandle;
    };

    class AudioPathChangeCommand : public Command {
    public:
        AudioPathChangeCommand(Entity entity, const std::string& oldPath, const std::string& newPath)
            : m_Entity(entity), m_OldPath(oldPath), m_NewPath(newPath) {}

        void Execute() override {
            m_Entity.GetComponent<AudioSourceComponent>().Path = m_NewPath;
        }

        void Undo() override {
            m_Entity.GetComponent<AudioSourceComponent>().Path = m_OldPath;
        }

        std::string GetName() const override { return "Change Audio Path"; }

    private:
        Entity m_Entity;
        std::string m_OldPath, m_NewPath;
    };

}
