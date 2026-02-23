#pragma once
#include "Layer.h"

class InputTestLayer : public Layer {
public:
    InputTestLayer() : Layer("InputTestLayer") {}
    void OnImGuiRender() override;
};
