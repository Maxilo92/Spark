#pragma once
#include "Scene.h"
#include <entt/entt.hpp>
#include <assert.h>

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        assert(!HasComponent<T>());
        return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent() {
        assert(HasComponent<T>());
        return m_Scene->m_Registry.get<T>(m_EntityHandle);
    }

    template<typename T>
    bool HasComponent() {
        return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
    }

    bool operator==(const Entity& other) const {
        return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
    }

    bool operator!=(const Entity& other) const {
        return !(*this == other);
    }

    operator bool() const { return m_EntityHandle != entt::null; }
    operator entt::entity() const { return m_EntityHandle; }
    operator uint32_t() const { return (uint32_t)m_EntityHandle; }

private:
    entt::entity m_EntityHandle{ entt::null };
    Scene* m_Scene{ nullptr };
};
