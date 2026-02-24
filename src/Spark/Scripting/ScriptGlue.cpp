#include "ScriptGlue.h"
#include "Input.h"
#include "Components.h"
#include "Log.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Spark {

	void ScriptGlue::RegisterComponents(sol::state& lua)
	{
		// Types
		lua.new_usertype<glm::vec2>("vec2",
			sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
			"x", &glm::vec2::x,
			"y", &glm::vec2::y
		);

		lua.new_usertype<glm::vec3>("vec3",
			sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
			"x", &glm::vec3::x,
			"y", &glm::vec3::y,
			"z", &glm::vec3::z
		);

		// Components
		lua.new_usertype<TransformComponent>("Transform",
			"Translation", sol::property(&TransformComponent::Translation, &TransformComponent::Translation),
			"Rotation", sol::property(&TransformComponent::Rotation, &TransformComponent::Rotation),
			"Scale", sol::property(&TransformComponent::Scale, &TransformComponent::Scale)
		);

		lua.new_usertype<SpriteRendererComponent>("SpriteRenderer",
			"Color", sol::property(&SpriteRendererComponent::Color, &SpriteRendererComponent::Color),
			"TextureHandle", sol::property(&SpriteRendererComponent::TextureHandle, &SpriteRendererComponent::TextureHandle)
		);

        lua.new_usertype<Spark::SceneCamera>("SceneCamera",
            "GetOrthographicSize", &Spark::SceneCamera::GetOrthographicSize,
            "SetOrthographicSize", &Spark::SceneCamera::SetOrthographicSize
        );

		lua.new_usertype<CameraComponent>("Camera",
			"Primary", sol::property(&CameraComponent::Primary, &CameraComponent::Primary),
            "SceneCamera", sol::property(&CameraComponent::Camera),
			"OrthographicSize", sol::property([](CameraComponent& cc) { return cc.Camera.GetOrthographicSize(); }, [](CameraComponent& cc, float size) { cc.Camera.SetOrthographicSize(size); }));

		lua.new_usertype<Rigidbody2DComponent>("Rigidbody2D",
			"SetLinearVelocity", &Rigidbody2DComponent::SetLinearVelocity,
			"GetLinearVelocity", &Rigidbody2DComponent::GetLinearVelocity,
			"ApplyLinearImpulse", sol::overload(
				sol::resolve<void(const glm::vec2&, bool)>(&Rigidbody2DComponent::ApplyLinearImpulse),
				sol::resolve<void(const glm::vec2&, const glm::vec2&, bool)>(&Rigidbody2DComponent::ApplyLinearImpulse)
			),
			"SetTransform", &Rigidbody2DComponent::SetTransform,
			"GetPosition", &Rigidbody2DComponent::GetPosition,
			"SetGravityScale", &Rigidbody2DComponent::SetGravityScale,
			"GetGravityScale", &Rigidbody2DComponent::GetGravityScale,
			"FixedRotation", &Rigidbody2DComponent::FixedRotation,
			"Type", &Rigidbody2DComponent::Type
		);

		lua.new_enum("BodyType",
			"Static", Rigidbody2DComponent::BodyType::Static,
			"Kinematic", Rigidbody2DComponent::BodyType::Kinematic,
			"Dynamic", Rigidbody2DComponent::BodyType::Dynamic
		);

		lua.new_usertype<AudioSourceComponent>("AudioSource",
			"Play", &AudioSourceComponent::Play,
			"Stop", &AudioSourceComponent::Stop,
			"IsPlaying", &AudioSourceComponent::IsPlaying,
			"SetVolume", &AudioSourceComponent::SetVolume,
			"SetPitch", &AudioSourceComponent::SetPitch,
			"Volume", &AudioSourceComponent::Volume,
			"Pitch", &AudioSourceComponent::Pitch,
			"Loop", &AudioSourceComponent::Loop
		);

		// Global Log
		lua.set_function("Log", [](const std::string& msg) { SP_INFO("[Lua] " + msg); });

		// Input
		auto input = lua["Input"].get_or_create<sol::table>();
		input.set_function("IsKeyPressed", [](int key) { return Input::IsKeyPressed(key); });
		input.set_function("IsMouseButtonPressed", [](int button) { return Input::IsMouseButtonPressed(button); });
		input.set_function("GetMousePosition", []() { return Input::GetMousePosition(); });

		// Key Constants
		auto keys = lua["Keys"].get_or_create<sol::table>();
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
		keys["Escape"] = GLFW_KEY_ESCAPE;
		keys["Enter"] = GLFW_KEY_ENTER;
	}

}
