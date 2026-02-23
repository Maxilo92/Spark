cmake_minimum_required(VERSION 3.10)
project(Spark)

# C++ Standard erzwingen
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui Dateien definieren
set(IMGUI_DIR imgui-1.92.6)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Mac-spezifische Frameworks
find_package(OpenGL REQUIRED)
find_library(GLFW_LIB glfw REQUIRED)

# Neue Dateipfade
set(ENGINE_SOURCES
    src/main.cpp
    src/Spark/Core/Window.cpp
    src/Spark/Core/Application.cpp
    src/Spark/Core/MacOSUtils.mm
    src/Spark/Core/ProjectManager.cpp
    src/Spark/Core/CrashHandler.cpp
    src/Spark/Core/AudioManager.cpp
    src/Spark/Core/LayerStack.cpp
    src/Spark/ImGui/ImGuiLayer.cpp
    src/Spark/Editor/DebuggerLayer.cpp
    src/Spark/Editor/TriangleLayer.cpp
    src/Spark/Renderer/Buffer.cpp
    src/Spark/Renderer/VertexArray.cpp
    src/Spark/Renderer/Shader.cpp
    src/Spark/Renderer/Renderer.cpp
    src/Spark/Renderer/Renderer2D.cpp
    src/Spark/Renderer/OrthographicCamera.cpp
    src/Spark/Renderer/SubTexture2D.cpp
    src/Spark/Renderer/Texture.cpp
    src/Spark/Renderer/Framebuffer.cpp
    src/Spark/Renderer/Scene.cpp
    src/Spark/Renderer/SceneSerializer.cpp
    src/Spark/Editor/FileViewerPanel.cpp
    src/Spark/Editor/ContentBrowserPanel.cpp
    src/Spark/Editor/ConsolePanel.cpp
    src/Spark/Editor/SceneHierarchyPanel.cpp
    src/Spark/Editor/PlanPanel.cpp
    src/Spark/Editor/InputTestLayer.cpp
    src/Spark/Core/UUID.cpp
    src/Spark/Core/Log.cpp
    src/Spark/Core/FileSystem.cpp
    src/Spark/Core/InstanceLock.cpp
    src/Spark/Assets/AssetManager.cpp
    src/Spark/Core/AudioManager.cpp
    src/Spark/Core/ScriptEngine.cpp
    libs/stb_image/stb_image.cpp
    libs/miniaudio/miniaudio.cpp
    libs/imguizmo/ImGuizmo.cpp
)

# yaml-cpp Sources
file(GLOB_RECURSE YAML_CPP_SOURCES "libs/yaml-cpp/src/*.cpp")

# Box2D Sources
file(GLOB_RECURSE BOX2D_SOURCES "libs/box2d/src/*.cpp")

# Lua Sources (Exclude lua.c and luac.c which contain main)
file(GLOB LUA_SOURCES "libs/lua/src/*.c")
list(REMOVE_ITEM LUA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/lua/src/lua.c")
list(REMOVE_ITEM LUA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/lua/src/luac.c")

add_library(Lua STATIC ${LUA_SOURCES})
target_include_directories(Lua PUBLIC libs/lua/src)
# Compile Lua as C++ to avoid linkage issues with sol2 or use extern "C"
set_target_properties(Lua PROPERTIES LINKER_LANGUAGE CXX)

add_executable(Spark 
    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${YAML_CPP_SOURCES}
    ${BOX2D_SOURCES}
)

# Suchpfade für Header
target_include_directories(Spark PRIVATE 
    src
    src/Spark/Core
    src/Spark/Events
    src/Spark/ImGui
    src/Spark/Editor
    src/Spark/Renderer
    src/Spark/Assets
    libs/glm
    libs/stb_image
    libs/entt/include
    libs/yaml-cpp/include
    libs/box2d/include
    libs/lua/src
    libs/sol2/include
    libs/miniaudio
    libs/imguizmo
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(Spark 
    ${GLFW_LIB} 
    ${OPENGL_LIBRARIES} 
    Lua
    "-framework Cocoa" 
    "-framework IOKit" 
    "-framework CoreVideo"
    "-framework CoreAudio"
    "-framework AudioToolbox"
    "-framework AudioUnit"
    "-framework CoreFoundation"
)
