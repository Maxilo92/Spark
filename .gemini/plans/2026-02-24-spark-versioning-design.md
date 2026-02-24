# Design Document: Spark Engine Versioning & Optimization

## 1. Problem Statement
The Spark Engine currently lacks a formalized versioning system and a structured way to track changes (Changelog). Furthermore, an intentional 1-second delay during the startup sequence unnecessarily slows down the development cycle. Finally, the local repository is not yet linked to its intended remote destination on GitHub, preventing collaborative work and remote backups.

## 2. Requirements

### Functional Requirements
- **Change Tracking**: A `CHANGELOG.md` file must be maintained in the root directory following the "Keep a Changelog" standard.
- **Versioning**: A central source of truth for the engine version must be accessible to the C++ codebase.
- **Startup Optimization**: The artificial 1-second delay in the `Application` constructor must be removed.
- **Repository Sync**: The local git repository must be configured with a remote and pushed to GitHub.

### Non-Functional Requirements
- **Standards**: Use Semantic Versioning (SemVer) starting at `v0.1.0`.
- **Consistency**: The version number should be reflected in the application window title and potentially other UI elements.
- **Quality**: Ensure the application still initializes correctly without the artificial delay.

### Constraints
- Must not break existing build systems (CMake).
- Must adhere to the project's modular architecture (Spark::Core).

## 3. Approach

### Selected Approach: Centralized Version Header (Approach 1)
We will create a `src/Spark/Core/Version.h` file containing macros for the version components and a combined string. This allows any part of the engine to access the version. The `Application` class will use this to update the window title.

### Alternatives Considered
- **Simple Documentation**: Only tracking version in `CHANGELOG.md`. Rejected because the engine itself wouldn't "know" its version, making it harder to display in UI or logs automatically.
- **CMake-driven Versioning**: Injecting the version via CMake defines. Rejected for this phase to keep the C++ side simpler and more transparent for now, though it's a good future improvement.

## 4. Architecture

### Version.h
```cpp
#pragma once

#define SPARK_VERSION_MAJOR 0
#define SPARK_VERSION_MINOR 1
#define SPARK_VERSION_PATCH 0

#define SPARK_VERSION_STR "0.1.0"
```

### Application Integration
- `Application::Application()`: Remove `usleep(1000000)`.
- `Application::UpdateWindowTitle()`: Change to include `SPARK_VERSION_STR`.

### Repository Configuration
- Add remote `origin` -> `https://github.com/Maxilo92/Spark`.

## 5. Agent Team
- **`coder`**: Creates `Version.h`, modifies `Application.cpp`, and initializes `CHANGELOG.md`.
- **`devops_engineer`**: Configures Git remote, stages all changes, commits, and pushes to GitHub.

## 6. Risk Assessment & Mitigation
- **Uncommitted Changes**: The repo has many uncommitted changes.
  - *Mitigation*: Perform a baseline commit before versioning changes to maintain a clean history.
- **Remote Conflicts**: The GitHub repo might not be empty.
  - *Mitigation*: Use standard push. If it fails due to existing content, alert the user before forcing.

## 7. Success Criteria
- [ ] `CHANGELOG.md` exists with `v0.1.0` entry.
- [ ] `Version.h` correctly defines `0.1.0`.
- [ ] Window title shows "Spark Engine v0.1.0".
- [ ] Engine starts immediately (delay removed).
- [ ] `git remote -v` shows the GitHub URL.
- [ ] Changes are pushed to GitHub.
