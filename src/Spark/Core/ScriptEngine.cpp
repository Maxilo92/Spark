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
                b2Body* body = (b2Body*)rb->RuntimeBody;
                body->SetAwake(true);
                body->ApplyForceToCenter({ force.x, force.y }, true);
            }
        },
        "ApplyImpulse", [](Rigidbody2DComponent* rb, const glm::vec2& impulse) {
            if (rb && rb->RuntimeBody) {
                b2Body* body = (b2Body*)rb->RuntimeBody;
                body->SetAwake(true);
                body->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
            }
        },
        "SetLinearVelocity", [](Rigidbody2DComponent* rb, const glm::vec2& velocity) {
            if (rb && rb->RuntimeBody) {
                b2Body* body = (b2Body*)rb->RuntimeBody;
                body->SetAwake(true);
                body->SetLinearVelocity({ velocity.x, velocity.y });
            }
        },
        "SetTransform", [](Rigidbody2DComponent* rb, const glm::vec2& pos) {
            if (rb && rb->RuntimeBody) {
                b2Body* body = (b2Body*)rb->RuntimeBody;
                body->SetTransform({ pos.x, pos.y }, body->GetAngle());
                body->SetAwake(true);
            }
        },
        "GetPosition", [](Rigidbody2DComponent* rb) -> glm::vec2 {
            if (rb && rb->RuntimeBody) {
                const b2Vec2& pos = ((b2Body*)rb->RuntimeBody)->GetPosition();
                return { pos.x, pos.y };
            }
            return { 0, 0 };
        },
        "GetLinearVelocity", [](Rigidbody2DComponent* rb) -> glm::vec2 {
            if (rb && rb->RuntimeBody) {
                const b2Vec2& vel = ((b2Body*)rb->RuntimeBody)->GetLinearVelocity();
                return { vel.x, vel.y };
            }
            return { 0, 0 };
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
        // 1. Check ImGui (Best for Editor Viewport)
        // Convert GLFW Key to ImGuiKey
        ImGuiKey imguiKey = (ImGuiKey)(ImGuiKey_NamedKey_BEGIN + (key - 32)); // Standard Offset
        // This is a bit hacky, so let's use a cleaner approach:
        
        bool pressed = false;
        if (ImGui::GetCurrentContext()) {
            // Mapping for common WASD
            if (key == GLFW_KEY_W) pressed = ImGui::IsKeyDown(ImGuiKey_W);
            else if (key == GLFW_KEY_S) pressed = ImGui::IsKeyDown(ImGuiKey_S);
            else if (key == GLFW_KEY_A) pressed = ImGui::IsKeyDown(ImGuiKey_A);
            else if (key == GLFW_KEY_D) pressed = ImGui::IsKeyDown(ImGuiKey_D);
            else if (key == GLFW_KEY_G) pressed = ImGui::IsKeyDown(ImGuiKey_G);
            else if (key == GLFW_KEY_SPACE) pressed = ImGui::IsKeyDown(ImGuiKey_Space);
            else pressed = Spark::Input::IsKeyPressed(key);
        } else {
            pressed = Spark::Input::IsKeyPressed(key);
        }

        if (pressed) {
            SP_INFO("[Input] Key " + std::to_string(key) + " is pressed!");
        }
        
        return pressed; 
    });
    input.set_function("IsMouseButtonPressed", [](int button) { return Spark::Input::IsMouseButtonPressed(button); });

    // Key Constants Table
    auto keys = m_Lua["Keys"].get_or_create<sol::table>();
    keys["W"] = GLFW_KEY_W;
    keys["S"] = GLFW_KEY_S;
    keys["A"] = GLFW_KEY_A;
    keys["D"] = GLFW_KEY_D;
    keys["G"] = GLFW_KEY_G;
    keys["Space"] = GLFW_KEY_SPACE;
    keys["Left"] = GLFW_KEY_LEFT;
    keys["Right"] = GLFW_KEY_RIGHT;
    keys["Up"] = GLFW_KEY_UP;
    keys["Down"] = GLFW_KEY_DOWN;
    keys["Space"] = GLFW_KEY_SPACE;
    keys["Escape"] = GLFW_KEY_ESCAPE;
    keys["Enter"] = GLFW_KEY_ENTER;

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
