#pragma once
#include "Layer.h"

class DebuggerLayer : public Layer {
public:
    DebuggerLayer() : Layer("DebuggerLayer") {}

    void OnImGuiRender() override;

private:
    bool m_ShowDemoWindow = false;
    bool m_ShowDebuggerWindow = true;
    bool m_ShowEngineInfo = true;
};
