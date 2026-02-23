#pragma once
#include "Layer.h"

class DebuggerLayer : public Layer {
public:
    DebuggerLayer() : Layer("DebuggerLayer") {}

    void OnAttach() override;
    void OnImGuiRender() override;
    void OnEvent(Event& event) override;
    void OnDetach() override;

private:
    void SaveEditorSettings();
    void LoadEditorSettings();

private:
    bool m_ShowDemoWindow = false;
    bool m_ShowDebuggerWindow = true;
    bool m_ShowInputTest = true;
};
