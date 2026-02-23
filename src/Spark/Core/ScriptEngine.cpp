#include "ScriptEngine.h"
#include "Log.h"
#include "Entity.h"
#include "Components.h"
#include "Input.h"
#include <box2d/box2d.h>
#include <iostream>

sol::state ScriptEngine::m_Lua;

void ScriptEngine::Init() {
    SP_INFO("Initializing Lua Script Engine...");
    m_Lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::table);

    // --- TYPES BINDING ---
    m_Lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    m_Lua.new_usertype<glm::vec2>("vec2",
        sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    // --- COMPONENT BINDING ---
    m_Lua.new_usertype<TransformComponent>("Transform",
        "Translation", &TransformComponent::Translation,
        "Rotation", &TransformComponent::Rotation,
        "Scale", &TransformComponent::Scale
    );

    // --- PHYSICS BINDING ---
    m_Lua.new_usertype<Rigidbody2DComponent>("Rigidbody2D",
        sol::no_constructor,
        "ApplyForce", [](Rigidbody2DComponent* rb, const glm::vec2& force) {
            if (rb && rb->RuntimeBody) {
                // SP_INFO("ApplyForce called: " + std::to_string(force.x) + ", " + std::to_string(force.y));
                ((b2Body*)rb->RuntimeBody)->ApplyForceToCenter({ force.x, force.y }, true);
            }
        },
        "ApplyImpulse", [](Rigidbody2DComponent* rb, const glm::vec2& impulse) {
            if (rb && rb->RuntimeBody) {
                // SP_INFO("ApplyImpulse called: " + std::to_string(impulse.x) + ", " + std::to_string(impulse.y));
                ((b2Body*)rb->RuntimeBody)->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
            }
        },
        "GetGravityScale", [](Rigidbody2DComponent* rb) -> float {
            if (rb && rb->RuntimeBody) return ((b2Body*)rb->RuntimeBody)->GetGravityScale();
            return 1.0f;
        },
        "SetGravityScale", [](Rigidbody2DComponent* rb, float scale) {
            if (rb && rb->RuntimeBody) {
                ((b2Body*)rb->RuntimeBody)->SetGravityScale(scale);
            }
        }
    );

    // --- INPUT BINDING ---
    auto input = m_Lua["Input"].get_or_create<sol::table>();
    input.set_function("IsKeyPressed", [](int key) { 
        bool pressed = Spark::Input::IsKeyPressed(key);
        if (pressed) {
            SP_INFO("[Lua] Key " + std::to_string(key) + " is PRESSED");
        }
        return pressed; 
    });
    input.set_function("IsMouseButtonPressed", [](int button) { return Spark::Input::IsMouseButtonPressed(button); });

    // --- ENTITY BINDING ---
    // Wir brauchen eine globale Registry oder Szenen-Pointer, um Komponenten zu finden.
    // In Scene::OnRuntimeStart setzen wir "entityID". 
    // Wir brauchen eine Hilfsfunktion, um Komponenten von IDs zu bekommen.
    // (Diese wird in der Scene gesetzt, wenn das Script geladen wird)

    // Beispiel-Binding: Log Funktion
    m_Lua.set_function("Log", [](const std::string& message) {
        SP_TRACE("[Lua]: " + message);
    });

    SP_INFO("Script Engine Initialized (Lua 5.4)");
}

void ScriptEngine::Shutdown() {
    // sol::state cleans up automatically
}

void ScriptEngine::OnUpdate(float dt) {
    // Hier können globale Skript-Updates laufen, oder Garbage Collection Schritte
    m_Lua.collect_garbage();
}
