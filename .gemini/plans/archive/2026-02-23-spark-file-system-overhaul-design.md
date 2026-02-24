# Design Document: Spark File-Centric Hub (Overhaul)
**Date**: 2026-02-23  
**Status**: Approved

## 1. Problem Statement
The current file management in Spark is functional but fragmented. The **Content Browser** lacks robust file operations like renaming, deletion, and template-based creation within a professional menu structure. The **File Viewer (Editor)** is basic, with limited shortcut support and a manual workflow that forces users to switch between tools or windows frequently. To make Spark a self-contained development environment, we need a unified, "file-first" system where the browser acts as the project's command center and the editor provides a seamless, shortcut-driven coding experience.

## 2. Requirements

### Functional Requirements
- **Content Browser (Hub):**
    - **Menu Bar**: Dedicated "File" and "Edit" menus. "File" includes "New Asset" (Submenu with Templates) and "Refresh". "Edit" includes "Rename", "Delete", and "Copy Path".
    - **Smart Templates**: Pre-configured templates for `.lua` (with boilerplate), `.scene` (standard YAML header), and `.txt`. A "Custom..." option for any extension.
    - **Rich Context Menu**: Right-click actions for folders (New Folder, New Script, Delete) and files (Open, Rename, Delete, Show in Finder).
    - **Safe File Operations**: Integrated deletion with a "Confirm Delete" modal and robust renaming that updates internal tracking.
- **File Editor (Companion):**
    - **Menu Bar**: "File" menu with Save (Cmd+S), Reload, Rename, and Close (Cmd+W).
    - **Context-Aware Shortcuts**: Keyboard shortcuts work globally but target the currently focused window (e.g., Cmd+S saves the active editor buffer).
    - **Enhanced Text Editor**: Support for Tab indentation, basic Find/Replace, and a status bar showing line counts and zoom levels.
    - **Multi-File Support**: The editor should be able to switch between or manage multiple open files (via tabs or a clean buffer system).

### Non-Functional Requirements
- **Performance**: Large directory trees in the browser should remain responsive.
- **UX**: Minimum friction between "thinking of a file" and "editing it."
- **Robustness**: Graceful handling of file system errors (e.g., read-only files or name collisions).

### Constraints
- Must use **ImGui** for all UI elements.
- Shortcuts must support macOS (**Cmd**) natively while providing a fallback for other platforms (**Ctrl**).
- File operations must use standard C++17 `std::filesystem`.

## 3. Architecture

### Key Components
1. **`ContentBrowserPanel` (The Hub)**: 
    - Acts as the primary entry point.
    - `DrawMenuBar()`: Implements the top-level window menu for file creation and management.
    - `DrawContentTable()`: Enhanced list view with icons/emojis and right-click context menus for every item.
    - `AssetTemplateSystem`: A new utility to manage file boilerplate (Lua scripts, Scenes, Text files) and prompt for names before creation.
2. **`FileViewerPanel` (The Companion Editor)**:
    - A high-performance text and asset viewer.
    - `BufferManager`: Handles loading, saving, and dirty-tracking (unsaved changes) for files.
    - `ShortcutHandler`: Intercepts `ImGuiIO` events to trigger `SaveFile()` or `ReloadFile()` when focused.
    - `EditorMenuBar()`: Provides quick access to common editor tasks (Find/Replace, Font Scale).
3. **`FileOperationService`**:
    - A static or singleton utility that encapsulates `std::filesystem` operations (Rename, Delete, Move).
    - Provides safe wrappers with error logging and confirmation callbacks.

### Data Flow
- **Open**: Browser (Double-Click) -> `FileViewerPanel::OpenFile(path)`.
- **Create**: Browser (Menu/Context) -> `AssetTemplateSystem::Create(type, name)` -> File created on disk -> Browser Refreshed.
- **Sync**: Editor (Save) -> `FileOperationService::Write(path, content)` -> Editor "Dirty" flag cleared.

## 4. Agent Team
- **`architect`**: Finalize `FileOperationService` interface and sync patterns.
- **`coder`**: Refactor `ContentBrowserPanel` and `FileViewerPanel`. Implement UI features.
- **`refactor`**: Consolidate file logic and buffer management.
- **`tester`**: Verify file operations and shortcut responsiveness.
- **`technical_writer`**: Update docs with the "No-Leave" workflow and shortcuts.

## 5. Risk Assessment & Success Criteria

### Risks & Mitigations
- **Data Loss (High)**: Implement a "Confirm Delete" modal and safe renaming.
- **Shortcut Conflicts (Medium)**: Use `ImGui::IsWindowFocused()` to strictly scope editor shortcuts.
- **Large Directory Latency (Low)**: Cache entries and refresh manually or via file-change notifications.

### Success Criteria
1. "No-Leave" Workflow for script creation, writing, saving, and execution.
2. Full File Ops (Rename/Delete) reliably working from Browser and Editor menus.
3. Smart Templates for Lua and Scenes with correct headers.
4. Fluid Shortcuts (Cmd+S, Cmd+N, Cmd+W) feeling responsive.
5. Professional UI with functional Menu Bars and Rich Context Menus.
