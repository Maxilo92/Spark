#pragma once
#include <memory>
#include <string>
#include "Scene.h"

class SceneSerializer {
public:
    SceneSerializer(const std::shared_ptr<Scene>& scene);

    void Serialize(const std::string& filepath);
    bool Deserialize(const std::string& filepath);

private:
    std::shared_ptr<Scene> m_Scene;
};
