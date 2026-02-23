#pragma once
#include <string>
#include <sol/sol.hpp>

class ScriptEngine {
public:
    static void Init();
    static void Shutdown();

    static void OnUpdate(float dt);

    static sol::state& GetState() { return m_Lua; }

private:
    static sol::state m_Lua;
};
