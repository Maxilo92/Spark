#pragma once
#include "Command.h"
#include "Entity.h"
#include "Components.h"

namespace Spark {

    class TransformCommand : public Command {
    public:
        TransformCommand(Entity entity, const glm::vec3& oldPos, const glm::vec3& oldRot, const glm::vec3& oldScale,
                         const glm::vec3& newPos, const glm::vec3& newRot, const glm::vec3& newScale)
            : m_Entity(entity), m_OldPos(oldPos), m_OldRot(oldRot), m_OldScale(oldScale),
              m_NewPos(newPos), m_NewRot(newRot), m_NewScale(newScale) {}

        void Execute() override {
            auto& tc = m_Entity.GetComponent<TransformComponent>();
            tc.Translation = m_NewPos;
            tc.Rotation = m_NewRot;
            tc.Scale = m_NewScale;
        }

        void Undo() override {
            auto& tc = m_Entity.GetComponent<TransformComponent>();
            tc.Translation = m_OldPos;
            tc.Rotation = m_OldRot;
            tc.Scale = m_OldScale;
        }

        std::string GetName() const override { return "Transform Entity"; }

    private:
        Entity m_Entity;
        glm::vec3 m_OldPos, m_OldRot, m_OldScale;
        glm::vec3 m_NewPos, m_NewRot, m_NewScale;
    };

}
