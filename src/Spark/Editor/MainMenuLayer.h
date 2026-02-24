#pragma once
#include "Layer.h"

namespace Spark {

    class MainMenuLayer : public Layer {
    public:
        MainMenuLayer();
        virtual ~MainMenuLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(float dt) override;
        virtual void OnImGuiRender() override;
    };

}
