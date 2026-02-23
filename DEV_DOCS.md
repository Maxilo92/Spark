# Spark Engine - Technische Entwickler-Dokumentation

Diese Dokumentation richtet sich an Entwickler, die Spark erweitern, neue Komponenten hinzufügen oder die Core-Systeme verstehen möchten.

---

## 1. Architektur-Übersicht

Spark folgt einer **Layer-basierten Architektur**. Die `Application` hält einen `LayerStack`.
- **Layers:** Werden von unten nach oben aktualisiert und gerendert. Typischerweise liegt hier die Spiellogik.
- **Overlays:** Liegen immer über den normalen Layern (z. B. ImGui, Debug-Menüs).

### Der Game Loop (Run-Zyklus)
In `Application::Run()` passiert pro Frame folgendes:
1. **Window-Clear:** `m_Window->Clear()` (OpenGL Color/Depth Buffer).
2. **Layer Update:** Alle Layer erhalten `OnUpdate(dt)` für Physik und Logik.
3. **ImGui Begin:** Globaler ImGui-Frame wird gestartet.
4. **ImGui Render:** Alle Layer erhalten `OnImGuiRender()` für UI-Elemente.
5. **ImGui End:** ImGui-Daten werden an die GPU gesendet.
6. **Buffer Swap:** `m_Window->OnUpdate()` tauscht die Front-/Back-Buffer.

---

## 2. Das Entity-Component-System (ECS)

Spark nutzt **EnTT**. Wir verwenden jedoch eine Wrapper-Klasse `Entity`, um den Umgang zu vereinfachen.

### Ein Entity erstellen
```cpp
Entity player = m_Scene->CreateEntity("Player");
player.AddComponent<TransformComponent>(glm::vec3(0, 5, 0));
```

### Sicherer Umgang mit Komponenten
**Wichtig:** Da die Engine bei fehlenden Komponenten über Assertions abstürzen kann, muss **immer** `HasComponent` geprüft werden:
```cpp
if (entity.HasComponent<SpriteRendererComponent>()) {
    auto& sprite = entity.GetComponent<SpriteRendererComponent>();
    // ...
}
```

### Eigene Komponenten hinzufügen
Komponenten sind einfache `structs` in `Components.h`. 
1. `struct` definieren.
2. Konstruktoren für `entt::emplace` bereitstellen.
3. In `SceneHierarchyPanel::DrawComponents` registrieren, damit sie im Editor sichtbar sind.

---

## 3. Rendering Pipeline

### Renderer2D (Batch Renderer)
Der `Renderer2D` ist auf Performance optimiert. Er sammelt Zeichenbefehle und sendet sie gebündelt an die GPU.
- **Max Quads:** Standardmäßig 10.000 pro Batch.
- **Texture Slots:** Unterstützt bis zu 16 Texturen gleichzeitig pro Batch.

### Framebuffers & Viewport
Die Engine rendert die Szene nicht direkt ins Fenster, sondern in einen `Framebuffer`. Das ImGui-Fenster "Viewport" liest diese Textur aus. Dies erlaubt:
- Editor-UI über der Spielgrafik.
- Post-Processing Effekte auf der Framebuffer-Textur.
- Dynamische Skalierung der Spielauflösung unabhängig von der Fenstergröße.

---

## 4. Asset & Projekt Management

### AssetManager & UUIDs
Jedes Asset (Textur, Sound) hat eine `AssetHandle` (64-Bit UUID).
- Verweise in Komponenten speichern niemals Pfade, sondern nur die `UUID`.
- Vorteil: Wenn eine Datei verschoben wird, muss nur die `AssetRegistry.yaml` aktualisiert werden; alle Szenen bleiben intakt.

### Projekt-Dateien (.spark)
Ein Projekt ist ein ZIP-Archiv.
- **Export:** `ProjectManager::SaveProject` nutzt den System-Befehl `zip`.
- **Struktur im ZIP:**
    - `assets/`: Alle Spielressourcen.
    - `plan.yaml`: Der Projekt-Fahrplan.
    - `project_meta.yaml`: Metadaten (Name, Version).

---

## 5. Scripting mit Lua

Wir nutzen **sol2** für die Anbindung.
- **Environment:** Jedes Skript erhält sein eigenes `sol::environment`, um globale Variablen-Kollisionen zu vermeiden.
- **Hooks:** Die Engine sucht nach `OnStart()` und `OnUpdate(dt)` Funktionen im Lua-Skript.
- **Bindings:** In `ScriptEngine.cpp` werden C++ Funktionen (wie Loggen oder Transform-Änderungen) für Lua registriert.

---

## 6. Fehlerbehandlung & Debugging

### Crash Reporting
Der `CrashHandler` fängt Signale ab.
- **Debugging:** Wenn die Engine abstürzt, öffnet sich ein macOS-Alert. Prüfe danach den neuesten Ordner unter `crashes/`.
- **Loggen:** Nutze `SP_INFO(...)`, `SP_WARN(...)` und `SP_ERROR(...)`. Diese erscheinen in der Datei `Spark.log` und im Editor-Konsolen-Panel.

---

## 7. Workflow-Guide: Ein neues Feature einbauen

Beispiel: Du möchtest ein "Health-System" einbauen.
1. **Komponente:** Erstelle `struct HealthComponent { float Value = 100.0f; }` in `Components.h`.
2. **Editor:** Füge in `SceneHierarchyPanel.cpp` bei `DrawComponents` ein `if (entity.HasComponent<HealthComponent>()) { ... ImGui::DragFloat(...) }` hinzu.
3. **Menü:** Füge "Health" zum "Add Component" Popup in `SceneHierarchyPanel.cpp` hinzu.
4. **Logik:** Erstelle einen neuen Layer oder bearbeite `Scene::OnUpdate`, um die Health-Werte zu verarbeiten.
5. **Serialisierung:** Füge die Komponente in `SceneSerializer.cpp` zu `SerializeEntity` und `Deserialize` hinzu, damit sie gespeichert wird.

---

## 8. File Management & Editor

Spark nutzt ein integriertes Hub-System für das Asset-Management und die Code-Bearbeitung.

### Der Content Browser (Hub)
Der Content Browser fungiert als zentrale Anlaufstelle für alle Dateioperationen.
- **Navigation:** Nutze den "Back"-Button oder die Breadcrumb-Leiste oben, um durch den `assets/`-Ordner zu navigieren.
- **Öffnen:** Ein Doppelklick auf einen Ordner öffnet diesen. Ein Doppelklick auf eine Datei öffnet den integrierten File Viewer/Editor.
- **Kontextmenü:** Ein Rechtsklick auf eine Datei oder in den leeren Bereich öffnet ein Menü für:
    - **New Asset...**: Erstellt neue Lua-Skripte, Szenen oder Textdateien mit vordefinierten Vorlagen.
    - **Rename (F2)**: Benennt die Datei oder den Ordner um.
    - **Delete (Del)**: Löscht das Element (mit Sicherheitsabfrage).
- **Drag & Drop:** Dateien können einfach per Maus in Ordner verschoben werden, um die Projektstruktur zu organisieren.

### Integrierter Editor (IDE Features)
Dateien wie `.lua`, `.scene`, `.yaml`, `.txt` und `.spark` können direkt in Spark bearbeitet werden.
- **Shortcuts:**
    - `Cmd+S`: Datei speichern.
    - `Cmd+W`: Editor-Tab schließen.
    - `Cmd+=` / `Cmd+-`: Zoom In / Zoom Out.
    - `Cmd+0`: Zoom zurücksetzen.
- **Features:**
    - **Find & Replace:** Suche nach Begriffen und ersetze sie global in der Datei.
    - **Live-Status:** Die Statusleiste zeigt Pfad, Zeilenanzahl, Dateigröße und den Speicherstatus ("Unsaved Changes") an.
    - **Previews:** Bilder (`.png`, `.jpg`) werden direkt angezeigt. Audio-Dateien (`.wav`, `.mp3`) können über einen "Play"-Button vorgehört werden.

### Technische Services
- **`FileOperationService`**: Bietet statische Methoden für sichere Dateioperationen (Rename, Delete, Move, CreateDirectory) inklusive Error-Logging.
- **`AssetTemplateSystem`**: Verwaltet die "Smart Templates". Es generiert Boilerplate-Code für neue Assets und stellt sicher, dass Dateinamen im Verzeichnis eindeutig sind (`GetUniqueName`).
