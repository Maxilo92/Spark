#include "LayerStack.h"
#include "Log.h"
#include <algorithm>

LayerStack::~LayerStack() {
    for (Layer* layer : m_Layers) {
        layer->OnDetach();
        delete layer;
    }
}

void LayerStack::PushLayer(Layer* layer) {
    SP_DEBUG_TRACE("LayerStack: Pushing Layer '" + layer->GetName() + "'");
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
    m_LayerInsertIndex++;
    layer->OnAttach();
}

void LayerStack::PushOverlay(Layer* overlay) {
    SP_DEBUG_TRACE("LayerStack: Pushing Overlay '" + overlay->GetName() + "'");
    m_Layers.emplace_back(overlay);
    overlay->OnAttach();
}

void LayerStack::PushOverlayManual(Layer* overlay) {
    m_Layers.emplace_back(overlay);
}

void LayerStack::PopLayer(Layer* layer) {
    auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
    if (it != m_Layers.begin() + m_LayerInsertIndex) {
        layer->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}

void LayerStack::PopOverlay(Layer* overlay) {
    auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
    if (it != m_Layers.end()) {
        overlay->OnDetach();
        m_Layers.erase(it);
    }
}

void LayerStack::Clear(bool keepOverlays) {
    if (keepOverlays) {
        // Only clear up to m_LayerInsertIndex
        for (uint32_t i = 0; i < m_LayerInsertIndex; i++) {
            m_Layers[i]->OnDetach();
            delete m_Layers[i];
        }
        m_Layers.erase(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex);
        m_LayerInsertIndex = 0;
    } else {
        for (Layer* layer : m_Layers) {
            layer->OnDetach();
            delete layer;
        }
        m_Layers.clear();
        m_LayerInsertIndex = 0;
    }
}
