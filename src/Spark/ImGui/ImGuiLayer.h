#pragma once
#include "Layer.h"
#include "Window.h"

class ImGuiLayer : public Layer {
public:
    ImGuiLayer(Window& window);
    ~ImGuiLayer() = default;

    void OnAttach() override;
    void OnDetach() override;
    void Begin();
    void End();

private:
    Window& m_Window;
};
