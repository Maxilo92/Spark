#pragma once
#include <entt/entt.hpp>
#include <string>
#include "UUID.h"
#include "OrthographicCamera.h"

class Entity;

class Scene {
public:
    Scene();
    ~Scene();

    Entity CreateEntity(const std::string& name = "Empty Entity");
    Entity CreateEntityWithUUID(Spark::UUID uuid, const std::string& name = "Empty Entity");
    void Clear();
    
    // Physik Simulation starten/stoppen
    void OnRuntimeStart();
    void OnRuntimeStop();

    static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> other);

    bool IsSimulating() const { return m_IsSimulating; }

    void OnUpdate(float dt);
    void Render(const OrthographicCamera& camera);

    entt::registry m_Registry;

private:
    friend class Entity;
    void* m_PhysicsWorld = nullptr; // b2World*
    bool m_IsSimulating = false;
};
