#pragma once
#include "Command.h"
#include "Entity.h"
#include "Components.h"

namespace Spark {

    template<typename T, typename Component>
    class ComponentPropertyCommand : public Command {
    public:
        using Setter = std::function<void(Component&, const T&)>;
        using Getter = std::function<T(const Component&)>;

        ComponentPropertyCommand(Entity entity, const std::string& name, const T& oldVal, const T& newVal, Setter setter)
            : m_Entity(entity), m_Name(name), m_OldVal(oldVal), m_NewVal(newVal), m_Setter(setter) {}

        void Execute() override {
            if (m_Entity.HasComponent<Component>()) {
                m_Setter(m_Entity.GetComponent<Component>(), m_NewVal);
            }
        }

        void Undo() override {
            if (m_Entity.HasComponent<Component>()) {
                m_Setter(m_Entity.GetComponent<Component>(), m_OldVal);
            }
        }

        std::string GetName() const override { return "Change Property: " + m_Name; }

    private:
        Entity m_Entity;
        std::string m_Name;
        T m_OldVal, m_NewVal;
        Setter m_Setter;
    };

}
