#pragma once
#include "Command.h"
#include "Entity.h"
#include "Scene.h"
#include "Components.h"

namespace Spark {

    class CreateEntityCommand : public Command {
    public:
        CreateEntityCommand(const std::shared_ptr<Scene>& scene, const std::string& name)
            : m_Scene(scene), m_Name(name) {}

        virtual void Execute() override {
            if (m_EntityID == entt::null) {
                m_Entity = m_Scene->CreateEntity(m_Name);
                m_EntityID = (entt::entity)m_Entity;
            } else {
                // Re-create with same ID (not simple in EnTT, usually we'd store component data)
                // For now, let's just create a new one and store the ID for future Undo
                m_Entity = m_Scene->CreateEntity(m_Name);
                m_EntityID = (entt::entity)m_Entity;
            }
        }

        virtual void Undo() override {
            m_Scene->DestroyEntity(m_Entity);
        }

        virtual std::string GetName() const override { return "Create Entity"; }

    private:
        std::shared_ptr<Scene> m_Scene;
        std::string m_Name;
        Entity m_Entity;
        entt::entity m_EntityID = entt::null;
    };

    // Note: A robust DeleteEntityCommand would need to store all component data to restore it!
    // This is a simplified version for demonstration.
}
