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
    void DestroyEntity(Entity entity);
    void Clear();
    
    // Physik Simulation starten/stoppen
    void OnRuntimeStart();
    void OnRuntimeStop();

    static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> other);

    bool IsSimulating() const { return m_IsSimulating; }

    void OnUpdate(float dt);
    void OnViewportResize(uint32_t width, uint32_t height);
    
    void Render(const glm::mat4& projection, const glm::mat4& transform);
    void Render(const OrthographicCamera& camera);
    void RenderRuntime();

    Entity GetPrimaryCameraEntity();

    entt::registry m_Registry;

    struct CollisionEvent {
        entt::entity Entity;
        entt::entity Other;
        bool Begin;
    };
    std::vector<CollisionEvent> m_CollisionQueue;

private:
    friend class Entity;
    void* m_PhysicsWorld = nullptr; // b2World*
    void* m_ContactListener = nullptr; // SparkContactListener*
    std::vector<entt::entity> m_EntitiesToDestroy;
    bool m_IsSimulating = false;
};
