#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "ScriptEngine.h"
#include "AudioManager.h"
#include "AssetManager.h"
#include "Renderer2D.h"
#include "miniaudio.h"
#include "Log.h"
#include <box2d/box2d.h>
#include <iostream>

namespace Spark {

    class SparkContactListener : public b2ContactListener {
    public:
        SparkContactListener(Scene* scene) : m_Scene(scene) {}

        void BeginContact(b2Contact* contact) override {
            entt::entity entityA = (entt::entity)contact->GetFixtureA()->GetUserData().pointer;
            entt::entity entityB = (entt::entity)contact->GetFixtureB()->GetUserData().pointer;

            if (m_Scene->m_Registry.valid(entityA))
                m_Scene->m_CollisionQueue.push_back({ entityA, entityB, true });
            if (m_Scene->m_Registry.valid(entityB))
                m_Scene->m_CollisionQueue.push_back({ entityB, entityA, true });
        }

        void EndContact(b2Contact* contact) override {
            entt::entity entityA = (entt::entity)contact->GetFixtureA()->GetUserData().pointer;
            entt::entity entityB = (entt::entity)contact->GetFixtureB()->GetUserData().pointer;

            if (m_Scene->m_Registry.valid(entityA))
                m_Scene->m_CollisionQueue.push_back({ entityA, entityB, false });
            if (m_Scene->m_Registry.valid(entityB))
                m_Scene->m_CollisionQueue.push_back({ entityB, entityA, false });
        }

    private:
        Scene* m_Scene;
    };

}

Scene::Scene() {
}

Scene::~Scene() {
    if (m_PhysicsWorld) delete (b2World*)m_PhysicsWorld;
}

std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> other) {
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    auto& srcRegistry = other->m_Registry;
    auto& dstRegistry = newScene->m_Registry;

    srcRegistry.view<IDComponent>().each([&](auto entity, auto& id) {
        std::string name = srcRegistry.get<TagComponent>(entity).Tag;
        Entity newEntity = newScene->CreateEntityWithUUID(id.ID, name);

        // Transform (Always present because CreateEntityWithUUID adds it)
        auto& tc = srcRegistry.get<TransformComponent>(entity);
        newEntity.GetComponent<TransformComponent>() = tc;

        // SpriteRenderer
        if (srcRegistry.all_of<SpriteRendererComponent>(entity)) {
            newEntity.AddComponent<SpriteRendererComponent>(srcRegistry.get<SpriteRendererComponent>(entity));
        }

        // CircleRenderer
        if (srcRegistry.all_of<CircleRendererComponent>(entity)) {
            newEntity.AddComponent<CircleRendererComponent>(srcRegistry.get<CircleRendererComponent>(entity));
        }

        // SpriteAnimation
        if (srcRegistry.all_of<SpriteAnimationComponent>(entity)) {
            newEntity.AddComponent<SpriteAnimationComponent>(srcRegistry.get<SpriteAnimationComponent>(entity));
        }

        // Rigidbody2D
        if (srcRegistry.all_of<Rigidbody2DComponent>(entity)) {
            auto& rb2d = newEntity.AddComponent<Rigidbody2DComponent>(srcRegistry.get<Rigidbody2DComponent>(entity));
            rb2d.RuntimeBody = nullptr;
        }

        // BoxCollider2D
        if (srcRegistry.all_of<BoxCollider2DComponent>(entity)) {
            auto& bc2d = newEntity.AddComponent<BoxCollider2DComponent>(srcRegistry.get<BoxCollider2DComponent>(entity));
            bc2d.RuntimeFixture = nullptr;
        }

        // CircleCollider2D
        if (srcRegistry.all_of<CircleCollider2DComponent>(entity)) {
            auto& cc2d = newEntity.AddComponent<CircleCollider2DComponent>(srcRegistry.get<CircleCollider2DComponent>(entity));
            cc2d.RuntimeFixture = nullptr;
        }

        // LuaScript
        if (srcRegistry.all_of<LuaScriptComponent>(entity)) {
            auto& lua = newEntity.AddComponent<LuaScriptComponent>(srcRegistry.get<LuaScriptComponent>(entity));
            lua.Initialized = false;
        }

        // AudioSource
        if (srcRegistry.all_of<AudioSourceComponent>(entity)) {
            auto& audio = newEntity.AddComponent<AudioSourceComponent>(srcRegistry.get<AudioSourceComponent>(entity));
            audio.RuntimeSound = nullptr;
        }

        // AudioListener
        if (srcRegistry.all_of<AudioListenerComponent>(entity)) {
            newEntity.AddComponent<AudioListenerComponent>(srcRegistry.get<AudioListenerComponent>(entity));
        }

        // Camera
        if (srcRegistry.all_of<CameraComponent>(entity)) {
            newEntity.AddComponent<CameraComponent>(srcRegistry.get<CameraComponent>(entity));
        }
    });

    return newScene;
}

Entity Scene::CreateEntity(const std::string& name) {
    return CreateEntityWithUUID(Spark::UUID(), name);
}

Entity Scene::CreateEntityWithUUID(Spark::UUID uuid, const std::string& name) {
    Entity entity(m_Registry.create(), this);
    entity.AddComponent<IDComponent>(uuid);
    entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
    entity.AddComponent<TransformComponent>();
    
    SP_DEBUG_TRACE("Scene: Created Entity '" + name + "' (UUID: " + std::to_string((uint64_t)uuid) + ")");
    return entity;
}

void Scene::DestroyEntity(Entity entity) {
    std::string name = "Unknown";
    if (entity.HasComponent<TagComponent>()) name = entity.GetComponent<TagComponent>().Tag;
    SP_DEBUG_TRACE("Scene: Destroying Entity '" + name + "'");

    // 1. Physik-Cleanup
    if (m_PhysicsWorld && entity.HasComponent<Rigidbody2DComponent>()) {
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        if (rb2d.RuntimeBody) {
            ((b2World*)m_PhysicsWorld)->DestroyBody((b2Body*)rb2d.RuntimeBody);
            rb2d.RuntimeBody = nullptr;
        }
    }

    // 2. Audio-Cleanup
    if (entity.HasComponent<AudioSourceComponent>()) {
        auto& audio = entity.GetComponent<AudioSourceComponent>();
        if (audio.RuntimeSound) {
            ma_sound* sound = (ma_sound*)audio.RuntimeSound;
            ma_sound_uninit(sound);
            delete sound;
            audio.RuntimeSound = nullptr;
        }
    }

    // 3. Registry-Cleanup
    m_Registry.destroy((entt::entity)entity);
}

void Scene::Clear() {
    m_Registry.clear();
}

void Scene::OnRuntimeStart() {
    SP_INFO("Scene Runtime Starting...");
    m_EntitiesToDestroy.clear();
    m_CollisionQueue.clear();
    m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
    
    // Contact Listener initialisieren
    m_ContactListener = new Spark::SparkContactListener(this);
    ((b2World*)m_PhysicsWorld)->SetContactListener((b2ContactListener*)m_ContactListener);

    // Lua Helper für das aktuelle Entity in diesem Scene-Kontext
    auto& m_Lua = ScriptEngine::GetState();
    m_Lua["Internal_GetRigidbody"] = [&](uint32_t id) -> Rigidbody2DComponent* {
        if (!m_Registry.valid((entt::entity)id)) return nullptr;
        if (!m_Registry.all_of<Rigidbody2DComponent>((entt::entity)id)) return nullptr;
        return &m_Registry.get<Rigidbody2DComponent>((entt::entity)id);
    };
    m_Lua["Internal_GetAudio"] = [&](uint32_t id) -> AudioSourceComponent* {
        if (!m_Registry.valid((entt::entity)id)) return nullptr;
        if (!m_Registry.all_of<AudioSourceComponent>((entt::entity)id)) return nullptr;
        return &m_Registry.get<AudioSourceComponent>((entt::entity)id);
    };
    m_Lua["Internal_GetTransform"] = [&](uint32_t id) -> TransformComponent* {
        if (!m_Registry.valid((entt::entity)id)) return nullptr;
        if (!m_Registry.all_of<TransformComponent>((entt::entity)id)) return nullptr;
        return &m_Registry.get<TransformComponent>((entt::entity)id);
    };
    m_Lua["Internal_GetCamera"] = [&](uint32_t id) -> CameraComponent* {
        if (!m_Registry.valid((entt::entity)id)) return nullptr;
        if (!m_Registry.all_of<CameraComponent>((entt::entity)id)) return nullptr;
        return &m_Registry.get<CameraComponent>((entt::entity)id);
    };
    m_Lua["Internal_GetTag"] = [&](uint32_t id) -> std::string {
        if (!m_Registry.valid((entt::entity)id)) return "Unknown";
        if (!m_Registry.all_of<TagComponent>((entt::entity)id)) return "Entity";
        return m_Registry.get<TagComponent>((entt::entity)id).Tag;
    };
    m_Lua["Internal_FindEntityByName"] = [&](const std::string& name) -> uint32_t {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view) {
            if (view.get<TagComponent>(entity).Tag == name)
                return (uint32_t)entity;
        }
        return 0; // Invalid ID
    };
    m_Lua["Internal_DestroyEntity"] = [&](uint32_t id) {
        if (!m_Registry.valid((entt::entity)id)) return;
        m_EntitiesToDestroy.push_back((entt::entity)id);
    };

    // 1. Physik initialisieren
    auto view = m_Registry.view<Rigidbody2DComponent>();
    for (auto e : view) {
        Entity entity = { e, this };
        auto& transform = entity.GetComponent<TransformComponent>();
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

        b2BodyDef bodyDef;
        bodyDef.type = (b2BodyType)rb2d.Type;
        bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
        bodyDef.angle = glm::radians(transform.Rotation.z);

        b2Body* body = ((b2World*)m_PhysicsWorld)->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        body->SetSleepingAllowed(false); // Ensure bodies are always responsive
        rb2d.RuntimeBody = body;

        if (entity.HasComponent<BoxCollider2DComponent>()) {
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

            b2PolygonShape boxShape;
            boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, { bc2d.Offset.x, bc2d.Offset.y }, 0.0f);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = bc2d.Density;
            fixtureDef.friction = bc2d.Friction;
            fixtureDef.restitution = bc2d.Restitution;
            fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
            fixtureDef.isSensor = bc2d.IsSensor;
            fixtureDef.userData.pointer = (uintptr_t)e;
            body->CreateFixture(&fixtureDef);
        }

        if (entity.HasComponent<CircleCollider2DComponent>()) {
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

            b2CircleShape circleShape;
            circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
            circleShape.m_radius = cc2d.Radius * transform.Scale.x;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = cc2d.Density;
            fixtureDef.friction = cc2d.Friction;
            fixtureDef.restitution = cc2d.Restitution;
            fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
            fixtureDef.isSensor = cc2d.IsSensor;
            fixtureDef.userData.pointer = (uintptr_t)e;
            body->CreateFixture(&fixtureDef);
        }

        body->ResetMassData();
    }

    // 2. Audio initialisieren
    auto audioView = m_Registry.view<AudioSourceComponent>();
    for (auto e : audioView) {
        Entity entity = { e, this };
        auto& audio = entity.GetComponent<AudioSourceComponent>();
        auto& transform = entity.GetComponent<TransformComponent>();

        if (!audio.Path.empty()) {
            ma_engine* engine = (ma_engine*)Spark::AudioManager::GetEngine();
            ma_sound* sound = new ma_sound();
            
            ma_uint32 flags = MA_SOUND_FLAG_DECODE;
            if (audio.Loop) flags |= MA_SOUND_FLAG_LOOPING;
            
            ma_result result = ma_sound_init_from_file(engine, audio.Path.c_str(), flags, NULL, NULL, sound);
            if (result == MA_SUCCESS) {
                audio.RuntimeSound = sound;
                ma_sound_set_volume(sound, audio.Volume);
                ma_sound_set_pitch(sound, audio.Pitch);
                if (audio.Spatial)
                    ma_sound_set_position(sound, transform.Translation.x, transform.Translation.y, transform.Translation.z);
                if (audio.PlayOnStart)
                    ma_sound_start(sound);
            } else {
                delete sound;
            }
        }
    }

    // 3. Lua Scripts initialisieren (Nachdem Physik/Audio bereit sind!)
    auto scriptView = m_Registry.view<LuaScriptComponent>();
    for (auto e : scriptView) {
        Entity entity = { e, this };
        auto& script = entity.GetComponent<LuaScriptComponent>();
        if (!script.Path.empty()) {
            SP_INFO("Loading script for entity: " + script.Path);
            script.Environment = sol::environment(ScriptEngine::GetState(), sol::create, ScriptEngine::GetState().globals());
            script.Environment["entityID"] = (uint32_t)e;

            try {
                ScriptEngine::GetState().script_file(script.Path, script.Environment);
                if (script.Environment["OnStart"].valid()) {
                    script.Environment["OnStart"]();
                }
                script.Initialized = true;
                SP_INFO("Script loaded successfully.");
            } catch (const sol::error& err) {
                SP_ERROR("Lua Error in " + script.Path + ": " + std::string(err.what()));
            }
        }
    }

    m_IsSimulating = true;
    SP_INFO("Scene Runtime Started.");
}

void Scene::OnRuntimeStop() {
    SP_INFO("Scene Runtime Stopping...");
    // Audio Cleanup
    auto audioView = m_Registry.view<AudioSourceComponent>();
    for (auto e : audioView) {
        auto& audio = audioView.get<AudioSourceComponent>(e);
        if (audio.RuntimeSound) {
            ma_sound* sound = (ma_sound*)audio.RuntimeSound;
            ma_sound_uninit(sound);
            delete sound;
            audio.RuntimeSound = nullptr;
        }
    }

    if (m_PhysicsWorld) {
        delete (b2World*)m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }

    if (m_ContactListener) {
        delete (b2ContactListener*)m_ContactListener;
        m_ContactListener = nullptr;
    }

    m_EntitiesToDestroy.clear();
    m_CollisionQueue.clear();

    // Reset runtime pointers
    m_Registry.view<Rigidbody2DComponent>().each([](auto entity, auto& rb2d) {
        rb2d.RuntimeBody = nullptr;
    });
    m_Registry.view<BoxCollider2DComponent>().each([](auto entity, auto& bc2d) {
        bc2d.RuntimeFixture = nullptr;
    });
    m_Registry.view<CircleCollider2DComponent>().each([](auto entity, auto& cc2d) {
        cc2d.RuntimeFixture = nullptr;
    });

    m_IsSimulating = false;
    SP_INFO("Scene Runtime Stopped.");
}

void Scene::OnUpdate(float dt) {
    if (m_IsSimulating) {
        // SP_INFO("Scene Update: Simulating with " + std::to_string(m_Registry.storage<entt::entity>().size()) + " entities");
        // Sprite Animation Update
        auto animView = m_Registry.view<SpriteRendererComponent, SpriteAnimationComponent>();
        for (auto e : animView) {
            auto& sprite = animView.get<SpriteRendererComponent>(e);
            auto& anim = animView.get<SpriteAnimationComponent>(e);
            
            if (anim.Playing && !anim.Frames.empty()) {
                anim.Timer += dt;
                if (anim.Timer >= anim.FrameTime) {
                    anim.Timer = 0.0f;
                    anim.CurrentFrame++;
                    
                    if (anim.CurrentFrame >= (int)anim.Frames.size()) {
                        if (anim.Loop) {
                            anim.CurrentFrame = 0;
                        } else {
                            anim.CurrentFrame = (int)anim.Frames.size() - 1;
                            anim.Playing = false;
                        }
                    }
                    
                    // Aktuelles Frame auf den Sprite Renderer übertragen
                    sprite.SubTexture = anim.Frames[anim.CurrentFrame];
                }
            }
        }

        // Audio Update (Positions)
        auto listenerView = m_Registry.view<TransformComponent, AudioListenerComponent>();
        for (auto e : listenerView) {
            auto& transform = listenerView.get<TransformComponent>(e);
            auto& listener = listenerView.get<AudioListenerComponent>(e);
            if (listener.Active) {
                ma_engine* engine = (ma_engine*)Spark::AudioManager::GetEngine();
                ma_engine_listener_set_position(engine, 0, transform.Translation.x, transform.Translation.y, transform.Translation.z);
                break;
            }
        }

        auto audioSourceView = m_Registry.view<TransformComponent, AudioSourceComponent>();
        for (auto e : audioSourceView) {
            auto& transform = audioSourceView.get<TransformComponent>(e);
            auto& audio = audioSourceView.get<AudioSourceComponent>(e);
            if (audio.RuntimeSound && audio.Spatial) {
                ma_sound_set_position((ma_sound*)audio.RuntimeSound, transform.Translation.x, transform.Translation.y, transform.Translation.z);
            }
        }

        // Lua Scripts Update
        auto scriptView = m_Registry.view<LuaScriptComponent>();
        for (auto e : scriptView) {
            auto& script = m_Registry.get<LuaScriptComponent>(e);
            if (script.Initialized && script.Environment["OnUpdate"].valid()) {
                try {
                    // SP_INFO("Updating script for entity " + std::to_string((uint32_t)e));
                    script.Environment["OnUpdate"](dt);
                } catch (const sol::error& err) {
                    SP_ERROR("Lua Runtime Error: " + std::string(err.what()));
                }
            } else if (!script.Initialized) {
                // SP_WARN("Script not initialized for entity " + std::to_string((uint32_t)e));
            }
        }

        const int32_t velocityIterations = 6;
        const int32_t positionIterations = 2;
        if (dt > 0.0f && m_PhysicsWorld) {
            ((b2World*)m_PhysicsWorld)->Step(dt, velocityIterations, positionIterations);
        }

        // Process deferred destruction
        for (auto e : m_EntitiesToDestroy) {
            if (m_Registry.valid(e)) {
                DestroyEntity({ e, this });
            }
        }
        m_EntitiesToDestroy.clear();

        // Process deferred collisions
        for (auto& e : m_CollisionQueue) {
            if (m_Registry.valid(e.Entity) && m_Registry.all_of<LuaScriptComponent>(e.Entity)) {
                auto& script = m_Registry.get<LuaScriptComponent>(e.Entity);
                if (script.Initialized) {
                    std::string funcName = e.Begin ? "OnCollisionBegin" : "OnCollisionEnd";
                    if (script.Environment[funcName].valid()) {
                        try {
                            script.Environment[funcName]((uint32_t)e.Other);
                        } catch (const sol::error& err) {
                            SP_ERROR("Lua Collision Error: " + std::string(err.what()));
                        }
                    }
                }
            }
        }
        m_CollisionQueue.clear();

        // Transform-Komponenten synchronisieren
        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view) {
            Entity entity = { e, this };
            if (!entity.HasComponent<TransformComponent>()) continue;
            
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            
            if (rb2d.RuntimeBody) {
                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                
                // SP_INFO("Syncing entity " + entity.GetComponent<TagComponent>().Tag + " to " + std::to_string(position.x) + ", " + std::to_string(position.y));
                
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = glm::degrees(body->GetAngle());
            }
        }
    }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view) {
        auto& cameraComponent = view.get<CameraComponent>(entity);
        if (!cameraComponent.FixedAspectRatio) {
            cameraComponent.Camera.SetViewportSize(width, height);
        }
    }
}

void Scene::Render(const glm::mat4& projection, const glm::mat4& transform) {
    Spark::Renderer2D::BeginScene(projection, transform);

    // Sprites
    auto spriteView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
    for (auto e : spriteView) {
        auto& tc = spriteView.get<TransformComponent>(e);
        auto& sprite = spriteView.get<SpriteRendererComponent>(e);
        
        if (sprite.SubTexture) {
            Spark::Renderer2D::DrawQuad(tc.GetTransform(), sprite.SubTexture, sprite.Color);
        } else if (sprite.TextureHandle != 0) {
            auto texture = Spark::AssetManager::GetAsset<Texture2D>(sprite.TextureHandle);
            if (texture) {
                Spark::Renderer2D::DrawQuad(tc.GetTransform(), texture, sprite.Color);
            }
        } else {
            Spark::Renderer2D::DrawQuad(tc.GetTransform(), sprite.Color);
        }
    }

    // Circles
    auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();
    for (auto e : circleView) {
        auto& tc = circleView.get<TransformComponent>(e);
        auto& circle = circleView.get<CircleRendererComponent>(e);
        
        Spark::Renderer2D::DrawCircle(tc.GetTransform(), circle.Color, circle.Thickness, circle.Fade);
    }

    Spark::Renderer2D::EndScene();
}

void Scene::Render(const OrthographicCamera& camera) {
    Spark::Renderer2D::BeginScene(camera);

    // Sprites
    auto spriteView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
    for (auto e : spriteView) {
        auto& tc = spriteView.get<TransformComponent>(e);
        auto& sprite = spriteView.get<SpriteRendererComponent>(e);
        
        if (sprite.SubTexture) {
            Spark::Renderer2D::DrawQuad(tc.GetTransform(), sprite.SubTexture, sprite.Color);
        } else if (sprite.TextureHandle != 0) {
            auto texture = Spark::AssetManager::GetAsset<Texture2D>(sprite.TextureHandle);
            if (texture) {
                Spark::Renderer2D::DrawQuad(tc.GetTransform(), texture, sprite.Color);
            }
        } else {
            Spark::Renderer2D::DrawQuad(tc.GetTransform(), sprite.Color);
        }
    }

    // Circles
    auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();
    for (auto e : circleView) {
        auto& tc = circleView.get<TransformComponent>(e);
        auto& circle = circleView.get<CircleRendererComponent>(e);
        
        Spark::Renderer2D::DrawCircle(tc.GetTransform(), circle.Color, circle.Thickness, circle.Fade);
    }

    Spark::Renderer2D::EndScene();
}

void Scene::RenderRuntime() {
    Entity cameraEntity = GetPrimaryCameraEntity();
    if (cameraEntity) {
        auto& cameraComp = cameraEntity.GetComponent<CameraComponent>();
        auto& transform = cameraEntity.GetComponent<TransformComponent>();

        // SP_INFO("Rendering with camera at " + std::to_string(transform.Translation.x) + ", " + std::to_string(transform.Translation.y));

        Render(cameraComp.Camera.GetProjection(), transform.GetTransform());
    } else {
        // SP_WARN("No primary camera found for runtime rendering!");
    }
}

Entity Scene::GetPrimaryCameraEntity()
{
    auto cameraView = m_Registry.view<CameraComponent>();
    for (auto entity : cameraView)
    {
        auto& camera = cameraView.get<CameraComponent>(entity);
        if (camera.Primary)
        {
            return { entity, this };
        }
    }
    return {};
}
