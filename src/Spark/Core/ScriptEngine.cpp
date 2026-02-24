#include "ScriptEngine.h"
#include "Spark/Scripting/ScriptGlue.h"
#include "Log.h"
#include <iostream>

sol::state ScriptEngine::m_Lua;

#include "Spark/Scripting/ScriptGlue.h"


void ScriptEngine::Init() {
    SP_INFO("Initializing Lua Script Engine...");
    m_Lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::table);

    Spark::ScriptGlue::RegisterComponents(m_Lua);

    SP_INFO("Script Engine Initialized (Lua 5.4)");
}

void ScriptEngine::Shutdown() {
    // sol::state cleans up automatically
}

void ScriptEngine::OnUpdate(float dt) {
    // Hier können globale Skript-Updates laufen, oder Garbage Collection Schritte
    m_Lua.collect_garbage();
}
