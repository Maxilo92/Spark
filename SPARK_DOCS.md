# Spark Engine - Comprehensive Documentation
**Version**: 1.0 (Developer Preview)  
**Last Updated**: 2026-02-23

> **Note**: This documentation is also accessible directly within the Spark Engine under the **Windows > Documentation** menu.

---

## Table of Contents
1. [Introduction](#1-introduction)
2. [Core Architecture](#2-core-architecture)
3. [The Entity-Component-System (ECS)](#3-the-entity-component-system-ecs)
4. [Rendering & Physics](#4-rendering--physics)
5. [Scripting (Lua)](#5-scripting-lua)
6. [Integrated Editor & Tools](#6-integrated-editor--tools)
7. [Asset & Project Management](#7-asset--project-management)
8. [Scripting API Reference](#8-scripting-api-reference)
9. [Component Reference](#9-component-reference)
10. [Development Workflow](#10-development-workflow)
11. [Roadmap & Future Work](#11-roadmap--future-work)

---

## 1. Introduction
**Spark** is a high-performance, modular 2D game engine built in C++17. It is designed to be a self-contained development environment ("The No-Leave Workflow"), where you can create, edit, test, and package your games without ever leaving the application.

### Key Technologies
*   **Renderer**: OpenGL 3.3+ (Batch Rendering).
*   **UI**: Dear ImGui (Docking, Themeable).
*   **ECS**: EnTT (Highly optimized).
*   **Scripting**: Lua 5.4 with `sol2`.
*   **Physics**: Box2D (Robust 2D simulation).
*   **Audio**: Miniaudio (Spatial & Multi-format).
*   **Build System**: Python & CMake.

---

## 2. Core Architecture
Spark uses a **Layer-based architecture** coordinated by the `Application` class.

### 2.1 The Application Class
The central controller that manages the window, the layer stack, and the main loop.
*   **LayerStack**: Layers are updated from bottom-to-top and rendered in reverse order for correct transparency.
*   **Overlays**: Special layers (like the Editor UI) that always render on top of game content.

### 2.2 The Main Loop (Frame Cycle)
Every frame follows this strict sequence:
1.  **Poll Events**: Input processing via GLFW.
2.  **OnUpdate(dt)**: Physics simulation, script logic, and camera movements.
3.  **OnImGuiRender()**: All UI panels (Hierarchy, Inspector, Viewport) are drawn.
4.  **Buffer Swap**: The GPU back-buffer is presented to the window.

---

## 3. The Entity-Component-System (ECS)
Spark utilizes **EnTT** for high-performance entity management. 

### 3.1 Entities & UUIDs
Every object in the scene is an `Entity`. 
*   **Identity**: Each entity has a unique 64-bit **UUID** (`IDComponent`) that persists across saves.
*   **Tags**: Human-readable names (`TagComponent`) for easy identification in the hierarchy.

### 3.2 Safe Component Access
Components are simple `structs`. To prevent crashes, always use `HasComponent` before access:
```cpp
if (entity.HasComponent<TransformComponent>()) {
    auto& transform = entity.GetComponent<TransformComponent>();
    // Modify transform...
}
```

---

## 4. Rendering & Physics

### 4.1 Renderer2D (Batching)
The engine uses a **Batch Renderer** to minimize Draw Calls.
*   **Max Quads**: Up to 10,000 per batch.
*   **Textures**: Supports up to 16 texture slots per batch (platform dependent).
*   **Primitives**: Fast rendering for Quads, Circles, and SubTextures (Sprite Sheets).

### 4.2 Physics (Box2D)
Integrated 2D rigid-body physics.
*   **Body Types**: Static, Kinematic, and Dynamic.
*   **Colliders**: Box and Circle colliders with customizable friction, density, and bounciness (restitution).
*   **Play Mode**: Scripts and physics only run when "Play" is pressed, operating on a temporary scene copy (Sandbox).

---

## 5. Scripting (Lua)
Spark allows game logic to be written in Lua without re-compiling the engine.

### 5.1 The Script Component
Add a `LuaScriptComponent` to any entity and point it to a `.lua` file. 
*   **Hot Reload**: (Planned) Changes to scripts are detected and applied instantly.
*   **Environment Isolation**: Each script runs in its own Lua environment to prevent variable leakage.

### 5.2 Hooks
The engine looks for specific functions in your Lua script:
*   `OnStart()`: Called when the scene starts or the entity is created.
*   `OnUpdate(dt)`: Called every frame during gameplay.

---

## 6. Integrated Editor & Tools
The Spark Editor is a professional-grade toolset built into the engine.

### 6.1 Content Browser (The Hub)
The central location for asset management.
*   **File Ops**: Right-click to Rename, Delete, or create New Folders.
*   **Asset Templates**: Quickly create boilerplates for Scripts, Scenes, and Text.
*   **Drag & Drop**: Move files between folders or drag assets into the Inspector.

### 6.2 Viewport & Gizmos
*   **Gizmos**: Visual manipulators for Translation, Rotation, and Scaling (powered by ImGuizmo).
*   **Play/Stop**: Instantly toggle between "Edit Mode" and "Game Mode".
*   **Shortcuts**: `Cmd+S` (Save Scene), `Cmd+N` (New File), `Cmd+W` (Close Tab).

### 6.3 Integrated IDE
A full text editor with features for coding:
*   **Search/Replace**: Global file searching.
*   **Multimedia Previews**: View textures and play audio files directly in the editor.
*   **Status Bar**: Tracks line count, zoom level, and unsaved changes.

---

## 7. Asset & Project Management

### 7.1 Asset Registry
Spark tracks assets using UUIDs in `AssetRegistry.yaml`. This decoupling ensures that moving files on disk doesn't break references in your scenes.

### 7.2 Project Format (.spark)
Projects are bundled into a single `.spark` archive (ZIP format).
*   **Backups**: Automated backups are created in `assets_backup/` when loading projects.
*   **Meta**: Metadata like name, version, and roadmap are stored inside the project.

---

## 8. Scripting API Reference

### 8.1 Basic Types
*   `vec3(x, y, z)`: 3D vector for transforms.
*   `vec2(x, y)`: 2D vector for physics forces/velocities.

### 8.2 Global Functions
*   `Log(message)`: Prints a trace message to the Engine Console.

### 8.3 Components
*   **Transform**: Access via `self.Transform`.
    *   `Translation`, `Rotation`, `Scale`.
*   **Rigidbody2D**:
    *   `ApplyForce(vec2)`: Apply continuous force.
    *   `ApplyImpulse(vec2)`: Apply instant velocity change.
    *   `SetLinearVelocity(vec2)`: Set velocity directly.
    *   `GetPosition()`: Current physics position.
    *   `SetGravityScale(float)`: Change gravity impact.

### 8.4 Input
*   `Input.IsKeyPressed(Keys.W)`: Returns true if key is held.
*   `Input.IsMouseButtonPressed(0)`: Returns true if left mouse is clicked.

---

## 9. Component Reference

| Component | Description | Key Properties |
| :--- | :--- | :--- |
| **Transform** | Position, Rotation, Scale | `Translation`, `Rotation`, `Scale` |
| **Sprite Renderer**| Renders a 2D quad | `Color`, `Texture`, `SubTexture` |
| **Circle Renderer**| Renders a 2D circle | `Color`, `Thickness`, `Fade` |
| **Lua Script** | Attaches code logic | `Path` (Path to .lua) |
| **Rigidbody2D** | Physics body | `Type` (Static/Dynamic), `FixedRotation` |
| **BoxCollider2D** | Rectangular collision | `Size`, `Offset`, `Friction` |
| **Audio Source** | Plays sound effects | `Path`, `Volume`, `Loop`, `Spatial` |
| **Tag** | Entity name | `Tag` (string) |

---

## 10. Development Workflow

### Building the Engine
Use the optimized Python script:
```bash
./build.py -r   # Build & Run
./build.py -cr  # Clean build & Run
```

### Adding a New Feature
1.  Define a **Component** in `Components.h`.
2.  Add **UI logic** in `SceneHierarchyPanel.cpp`.
3.  Implement **Serialisation** in `SceneSerializer.cpp`.
4.  Add **Lua Bindings** in `ScriptEngine.cpp` (optional).

---

## 11. Roadmap & Future Work
*   **Particle Systems**: High-performance GPU-based particles.
*   **Prefabs**: Reusable entity templates with overrides.
*   **Shader Editor**: Visual node-based shader graph.
*   **Tilemaps**: Dedicated editor for grid-based levels.
*   **Web Support**: Compiling for the browser via Emscripten.

---
*Created by the Spark Development Team.*
