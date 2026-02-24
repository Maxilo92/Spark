## ScriptGlue Design Document

This document outlines the design for the Lua Scripting API, focusing on component bindings and input.

### 1. Components

#### 1.1 TransformComponent

```cpp
m_Lua.new_usertype<TransformComponent>("Transform",
    "Translation", sol::property(&TransformComponent::Translation, &TransformComponent::Translation),
    "Rotation", sol::property(&TransformComponent::Rotation, &TransformComponent::Rotation),
    "Scale", sol::property(&TransformComponent::Scale, &TransformComponent::Scale)
);
```

Lua Usage:

```lua
entity.Transform.Translation = { x = 1.0, y = 2.0, z = 3.0 }
local x = entity.Transform.Translation.x
```

#### 1.2 SpriteRendererComponent

```cpp
m_Lua.new_usertype<SpriteRendererComponent>("SpriteRenderer",
    "Color", sol::property(&SpriteRendererComponent::Color, &SpriteRendererComponent::Color),
    "TextureHandle", sol::property(&SpriteRendererComponent::TextureHandle, &SpriteRendererComponent::TextureHandle)
);
```

Lua Usage:

```lua
entity.SpriteRenderer.Color = { x = 1.0, y = 0.0, z = 0.0, w = 1.0 }
local handle = entity.SpriteRenderer.TextureHandle
```

#### 1.3 CameraComponent

```cpp
m_Lua.new_usertype<CameraComponent>("Camera",
    "Primary", sol::property(&CameraComponent::Primary, &CameraComponent::Primary),
    "OrthoSize", sol::property(&CameraComponent::OrthoSize, &CameraComponent::OrthoSize)
);
```

Lua Usage:

```lua
entity.Camera.Primary = true
local size = entity.Camera.OrthoSize
```

### 2. Input

```cpp
auto input = m_Lua["Input"].get_or_create<sol::table>();
input.set_function("IsKeyPressed", [](int key) { return Spark::Input::IsKeyPressed(key); });
input.set_function("IsMouseButtonPressed", [](int button) { return Spark::Input::IsMouseButtonPressed(button); });
```

Lua Usage:

```lua
if Input.IsKeyPressed(Keys.Space) then
    Log("Space is pressed!")
end

if Input.IsMouseButtonPressed(0) then
    Log("Left mouse button is pressed!")
end
```

### 3. Key Constants

```cpp
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
```

Lua Usage:

```lua
if Input.IsKeyPressed(Keys.Space) then
    --...
end
```