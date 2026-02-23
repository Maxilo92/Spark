# Spark Engine - Architektur & Features

Dies ist eine modulare 2D C++ Game Engine, die auf **GLFW**, **OpenGL 3.3+** und **Dear ImGui** basiert. Die Architektur folgt einem objektorientierten Ansatz mit strikter Trennung von Plattform-Logik, Event-Verarbeitung und Funktionalität.

## 1. Kern-Systeme

### `Application` (Zentrale Steuerung)
Die `Application`-Klasse koordiniert die Hauptschleife (Main Loop), das Fenster (`Window`) und den `LayerStack`.
- **Main Loop:** In jedem Frame werden Updates (`OnUpdate`) und UI-Rendering (`OnImGuiRender`) ausgeführt.
- **Initialisierung:** Garantiert eine korrekte Reihenfolge (Fenster vor Renderer-Kontext), um Abstürze zu vermeiden.

### `Project Management` (Neu!)
Spark unterstützt ein vollständiges Projekt-Speichersystem.
- **`.spark` Format:** Projekte werden als komprimierte ZIP-Archive gespeichert, die den gesamten `assets/`-Ordner, die Roadmap und Metadaten enthalten.
- **Backup-System:** Beim Laden eines Projekts wird der aktuelle Arbeitsstand automatisch als `assets_backup/` gesichert.
- **Menü-Integration:** Über das **Project**-Menü können Projekte erstellt, gespeichert und geladen werden.

### `Crash Reporting & Diagnostics` (Neu!)
Ein robustes System zur Fehlerdiagnose bei Systemabstürzen.
- **Signal Handling:** Fängt kritische Signale wie `SIGSEGV` (Segmentation Fault) oder `SIGABRT` ab.
- **Crash Reports:** Erstellt bei einem Absturz automatisch einen Ordner `crashes/crash_[timestamp]/` mit:
    - `report.txt`: Details zum auslösenden Signal.
    - `Example.scene`: Der letzte Zustand der Welt.
    - `plan.yaml`: Der aktuelle Projektplan.
    - `Spark.log`: Die letzten Log-Einträge vor dem Fehler.

### `Build System` (Optimiert!)
Ein Python-basiertes Skript (`build.py`) vereinfacht den Workflow erheblich:
- `./build.py -r`: Kompiliert und startet die Engine aus dem Stammverzeichnis.
- `./build.py -c`: Führt einen sauberen ("Clean") Build durch.
- Automatische Pfad-Auflösung: Stellt sicher, dass Assets immer gefunden werden, egal aus welchem Ordner gestartet wird.

---

## 2. Editor & Tools

Die Engine verfügt über ein integriertes Editor-Tooling auf **ImGui**-Basis:

### `Plan & Roadmap Panel` (Neu!)
Ein integriertes Projektmanagement-Tool direkt in der Engine.
- **Gruppierung:** Aufgaben werden nach Kategorien (Core, Renderer, Editor) sortiert.
- **Priorisierung:** Farblich markierte Prioritätsstufen (High, Medium, Low).
- **Fortschritt:** Visualisierung des Projektfortschritts durch Fortschrittsbalken.
- **Filter & Suche:** Schnelles Finden von Aufgaben und Ausblenden erledigter Tasks.
- **Persistenz:** Speicherung in `assets/plan.yaml` via YAML.

### `Viewport & Gizmos` (Verbessert!)
- **ImGuizmo:** Erlaubt das visuelle Manipulieren von Entities (Verschieben, Rotieren, Skalieren) direkt im Viewport.
- **Undo/Redo:** Unterstützung für Rückgängig-Aktionen bei Transformationen.
- **Play/Stop:** Startet die physikalische Simulation und Skripte in einer Kopie der Szene (Sandbox-Modus).

### Weitere Panels
- **Scene Hierarchy:** Struktur der aktuellen Welt.
- **Properties:** Detail-Einstellungen für Komponenten.
- **Content Browser:** Datei-Explorer für Assets.
- **Console:** Echtzeit-Logs der Engine.

---

## 3. Technische Highlights

- **ECS (EnTT):** Hochperformantes Entity-Component-System. Jedes Entity hat eine stabile **UUID**.
- **Renderer2D:** Batch-Rendering für hunderte Objekte in einem Draw-Call.
- **Physik (Box2D):** Integrierte 2D-Physik mit Rigidbodies und Collidern.
- **Scripting (Lua/sol2):** Logik-Programmierung ohne Neukompilierung.
- **Audio (miniaudio):** Support für Spatial Audio und Sound-Effekte.
- **VFS:** Virtuelles Dateisystem für plattformunabhängige Pfade.

---

## 4. Schnellstart

Um die Engine zu bauen und zu starten, nutze das mitgelieferte Python-Skript:

```bash
# Bauen und Starten
./build.py -r

# Komplett neu bauen
./build.py -cr
```

Alternativ via CMake:
```bash
mkdir build && cd build
cmake ..
make
cd ..
./build/Spark
```
