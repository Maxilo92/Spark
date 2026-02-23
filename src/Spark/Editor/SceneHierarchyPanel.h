#pragma once
#include <memory>
#include "Scene.h"
#include "Entity.h"

class SceneHierarchyPanel {
public:
    SceneHierarchyPanel() = default;
    SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

    void SetContext(const std::shared_ptr<Scene>& scene);
    void OnImGuiRender(bool* p_open_hierarchy = nullptr, bool* p_open_properties = nullptr);

    Entity GetSelectedEntity() const { return m_SelectionContext; }

private:
    void DrawEntityNode(Entity entity);
    void DrawComponents(Entity entity);

private:
    std::shared_ptr<Scene> m_Context;
    Entity m_SelectionContext;
    friend class Scene;
};
