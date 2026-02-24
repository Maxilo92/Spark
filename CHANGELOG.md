# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.3] - 2026-02-24

### Added
- Integrated `ContentBrowserPanel` for browsing and managing assets within the editor.
- Implemented Drag & Drop integration between Content Browser and Viewport.
- Support for contextual dropping of textures, scripts, and scenes.

## [0.1.2] - 2026-02-24

## [0.1.1] - 2026-02-24

### Added
- `CameraComponent` for entities with projection and priority settings.
- `Scene::GetPrimaryCameraEntity()` to locate the active camera in a scene.
- `Scene::RenderRuntime()` for rendering using the primary camera's view and projection.

### Changed
- Improved `Scene::Render()` to be a clean rendering pass that accepts an `OrthographicCamera`.
- Updated `Scene::Copy()` to include `CameraComponent` when duplicating scenes.

## [0.1.0] - 2026-02-24

### Added
- Centralized versioning system in `src/Spark/Core/Version.h`.
- Initial `CHANGELOG.md` to track project progress.
- Git remote configuration for GitHub synchronization.

### Changed
- Updated application window title to display engine version.
- Removed artificial 1-second delay from startup sequence for faster initialization.
