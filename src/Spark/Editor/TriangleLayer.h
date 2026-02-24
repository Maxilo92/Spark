#pragma once
#include "Layer.h"
#include "Renderer.h"
#include "OrthographicCamera.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Scene.h"
#include "SceneHierarchyPanel.h"
#include "ContentBrowserPanel.h"
#include "FileViewerPanel.h"
#include "ConsolePanel.h"
#include "PlanPanel.h"
#include "ChangelogPanel.h"
#include "Command.h"
#include "TransformCommand.h"
#include <memory>

class TriangleLayer : public Layer {
public:
    TriangleLayer() : Layer("TriangleLayer"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f) {}

    void OnAttach() override;
    void OnUpdate(float dt) override;
    void OnImGuiRender() override;
    void OnEvent(Event& event) override;
    void OnDetach() override;

private:
    void SaveEditorSettings();
    void LoadEditorSettings();

private:
    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<Texture2D> m_Texture;
    std::shared_ptr<Framebuffer> m_Framebuffer;
    
    std::shared_ptr<Scene> m_ActiveScene;
    std::shared_ptr<Scene> m_EditorScene;
    SceneHierarchyPanel m_SceneHierarchyPanel;
    Spark::ContentBrowserPanel m_ContentBrowserPanel;
    Spark::FileViewerPanel m_FileViewerPanel;
    Spark::ConsolePanel m_ConsolePanel;
    PlanPanel m_PlanPanel;
    Spark::ChangelogPanel m_ChangelogPanel;
    
    // Window Visibility
    bool m_ShowViewport = true;
    bool m_ShowSceneHierarchy = true;
    bool m_ShowProperties = true;
    bool m_ShowContentBrowser = true;
    bool m_ShowFileViewer = true;
    bool m_ShowConsole = true;
    bool m_ShowPlanPanel = true;
    bool m_ShowSettings = true;
    bool m_ShowChangelog = false;

    bool m_ShowNewProjectPopup = false;
    bool m_ShowSaveProjectPopup = false;
    bool m_ShowLoadProjectPopup = false;
    bool m_ShowAboutPopup = false;
    char m_NewProjectNameBuffer[128] = "MyNewGame";
    char m_SaveProjectNameBuffer[128] = "MyGame";

    OrthographicCamera m_Camera;
    float m_Rotation = 0.0f;
    glm::vec2 m_ViewportSize = { 1280, 720 };
    glm::vec2 m_ViewportRectMin = { 0.0f, 0.0f };

    struct InitialTransform {
        glm::vec3 Translation, Rotation, Scale;
    } m_InitialTransform;
};
