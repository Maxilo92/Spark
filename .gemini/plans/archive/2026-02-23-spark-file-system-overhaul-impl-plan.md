# Implementation Plan: Spark File-Centric Hub Overhaul
**Date**: 2026-02-23  
**Status**: Pending Approval

## 1. Plan Overview
This plan outlines the staged implementation of a unified file management system for the Spark engine. The focus is on making the **Content Browser** the primary hub and the **File Viewer** a shortcut-driven, professional editor.

- **Total Phases**: 4
- **Agents Involved**: `architect`, `coder`, `refactor`, `tester`, `technical_writer`
- **Estimated Effort**: High

## 2. Dependency Graph
```mermaid
graph TD
    P1[Phase 1: Foundation & Services] --> P2[Phase 2: Content Browser Overhaul]
    P1 --> P3[Phase 3: File Viewer (Editor) Upgrade]
    P2 --> P4[Phase 4: Integration & Validation]
    P3 --> P4
    P4 --> P5[Phase 5: Documentation]
```

## 3. Execution Strategy Table
| Phase | Objective | Agent | Parallel |
| :--- | :--- | :--- | :--- |
| **1** | Core Services & Templates | `architect`, `coder` | No |
| **2** | Browser UI & Context Menus | `coder` | Yes (with P3) |
| **3** | Editor UI & Shortcuts | `coder` | Yes (with P2) |
| **4** | Integration & Sync | `refactor`, `tester` | No |
| **5** | Documentation | `technical_writer` | No |

## 4. Phase Details

### Phase 1: Foundation & Services
- **Objective**: Create the underlying services for safe file operations and asset templates.
- **Agent Assignment**: `architect` (Design), `coder` (Implementation)
- **Files to Create**:
    - `src/Spark/Editor/FileOperationService.h/cpp`: Static utility for Rename, Delete, Move with error handling.
    - `src/Spark/Editor/AssetTemplateSystem.h/cpp`: Logic for generating Lua, Scene, and Text boilerplate.
- **Validation**: Compile check and unit tests for `FileOperationService` (creating/deleting temp files).

### Phase 2: Content Browser Overhaul
- **Objective**: Implement the Menu Bar, Context Menus, and Template integration in the Browser.
- **Agent Assignment**: `coder`
- **Files to Modify**:
    - `src/Spark/Editor/ContentBrowserPanel.h/cpp`:
        - Add `DrawMenuBar()`.
        - Add "New Asset" submenu with templates.
        - Add rich Context Menus for files and folders.
        - Implement "Confirm Delete" modal.
- **Validation**: Manual verification of file creation, deletion, and renaming via the Browser UI.

### Phase 3: File Viewer (Editor) Upgrade
- **Objective**: Add Menu Bar, global shortcuts (Cmd+S), and buffer management to the Editor.
- **Agent Assignment**: `coder`
- **Files to Modify**:
    - `src/Spark/Editor/FileViewerPanel.h/cpp`:
        - Implement `EditorMenuBar()`.
        - Add `ShortcutHandler` for Cmd+S, Cmd+N, Cmd+W.
        - Enhance text editor with Tab support and status bar.
        - Implement dirty-tracking (Save check).
- **Validation**: Verify shortcuts trigger Save/Reload and that the Menu Bar functions correctly.

### Phase 4: Integration & Validation
- **Objective**: Ensure seamless sync between Browser and Editor and finalize the "No-Leave" workflow.
- **Agent Assignment**: `refactor` (Cleanup), `tester` (Verification)
- **Files to Modify**:
    - `src/Spark/Editor/TriangleLayer.cpp`: Ensure proper panel initialization and shortcut routing.
- **Validation**: Full "End-to-End" test: Create script -> Edit -> Save -> Rename -> Delete.

### Phase 5: Documentation
- **Objective**: Document the new workflow and shortcuts.
- **Agent Assignment**: `technical_writer`
- **Files to Modify**:
    - `DEV_DOCS.md`: Add "File Management & Editor" section.
    - `README.md`: Highlight new integrated dev environment features.

## 5. File Inventory
| File Path | Phase | Action | Purpose |
| :--- | :--- | :--- | :--- |
| `src/Spark/Editor/FileOperationService.h/cpp` | 1 | Create | Core file API |
| `src/Spark/Editor/AssetTemplateSystem.h/cpp` | 1 | Create | Boilerplate generator |
| `src/Spark/Editor/ContentBrowserPanel.cpp` | 2 | Modify | Browser UI Logic |
| `src/Spark/Editor/FileViewerPanel.cpp` | 3 | Modify | Editor UI Logic |
| `src/Spark/Editor/TriangleLayer.cpp` | 4 | Modify | Integration |

## 6. Execution Profile
- **Total phases**: 5
- **Parallelizable phases**: 2 (Phase 2 & 3)
- **Estimated sequential wall time**: ~2-3 hours
