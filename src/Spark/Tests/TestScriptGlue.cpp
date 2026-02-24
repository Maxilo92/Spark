#include <gtest/gtest.h>
#include <Spark/Core/ScriptEngine.h>
#include <Spark/Renderer/Components.h>
#include <glm/glm.hpp>

TEST(ScriptGlue, Transform)
{
    ScriptEngine::Init();
    sol::state& lua = ScriptEngine::GetState();

    lua.script("transform = { Translation = { x = 1.0, y = 2.0, z = 3.0 }, Rotation = { x = 4.0, y = 5.0, z = 6.0 }, Scale = { x = 7.0, y = 8.0, z = 9.0 } }");

    glm::vec3 translation = lua["transform"]["Translation"].get<glm::vec3>();
    glm::vec3 rotation = lua["transform"]["Rotation"].get<glm::vec3>();
    glm::vec3 scale = lua["transform"]["Scale"].get<glm::vec3>();

    ASSERT_EQ(translation.x, 1.0f);
    ASSERT_EQ(translation.y, 2.0f);
    ASSERT_EQ(translation.z, 3.0f);

    ASSERT_EQ(rotation.x, 4.0f);
    ASSERT_EQ(rotation.y, 5.0f);
    ASSERT_EQ(rotation.z, 6.0f);

    ASSERT_EQ(scale.x, 7.0f);
    ASSERT_EQ(scale.y, 8.0f);
    ASSERT_EQ(scale.z, 9.0f);

    ScriptEngine::Shutdown();
}

TEST(ScriptGlue, SpriteRenderer)
{
    ScriptEngine::Init();
    sol::state& lua = ScriptEngine::GetState();

    lua.script("sprite = { Color = { x = 0.1, y = 0.2, z = 0.3, w = 0.4 }, TextureHandle = 12345 }");

    glm::vec4 color = lua["sprite"]["Color"].get<glm::vec4>();
    Spark::AssetHandle handle = lua["sprite"]["TextureHandle"].get<Spark::AssetHandle>();

    ASSERT_EQ(color.x, 0.1f);
    ASSERT_EQ(color.y, 0.2f);
    ASSERT_EQ(color.z, 0.3f);
    ASSERT_EQ(color.w, 0.4f);

    ASSERT_EQ(handle, 12345);

    ScriptEngine::Shutdown();
}

TEST(ScriptGlue, Camera)
{
    ScriptEngine::Init();
    sol::state& lua = ScriptEngine::GetState();

    lua.script("camera = { Primary = true, OrthoSize = 5.0 }");

    bool primary = lua["camera"]["Primary"].get<bool>();
    float size = lua["camera"]["OrthoSize"].get<float>();

    ASSERT_EQ(primary, true);
    ASSERT_EQ(size, 5.0f);

    ScriptEngine::Shutdown();
}
