# Projekt-Richtlinien für KI-Agenten (Spark Engine)

Dieses Dokument enthält verbindliche Anweisungen für die Zusammenarbeit an der Spark Engine. Jeder KI-Agent MUSS diese Regeln befolgen.

## 1. ImGui Best Practices (CRITICAL)
Die Engine nutzt Dear ImGui für das Editor-Interface. Um Fehler bei der UI-Implementierung, falsche Widget-Nutzung oder Speicherlecks zu vermeiden, gilt folgende Regel:

- **Bevor** neue Features implementiert oder Bugs im UI-Bereich behoben werden, muss zwingend die Datei `imgui-1.92.6/imgui_demo.cpp` konsultiert werden.
- Die `imgui_demo.cpp` ist die Referenzquelle für die korrekte Nutzung aller ImGui-Funktionen.
- Suche dort nach ähnlichen Implementierungen (z.B. Log-Fenster, Menübars, Filter-Systeme), um den "ImGui-Way" einzuhalten.

## 2. Architektur-Prinzipien & Modulare Struktur
Die Spark Engine ist modular aufgebaut. Neue Features müssen in diese Struktur passen:

- **Schichtentrennung (LayerStack):** Nutze das `Layer`-System (`src/Spark/Core/Layer.h`), um Funktionalitäten zu trennen. Editor-Tools, Debug-Overlays und das eigentliche Spiel sollten eigene Layer sein. Dies erlaubt ein einfaches Ein-/Ausschalten von Modulen.
- **ECS (EnTT):** Trenne Daten streng von Logik.
    - **Komponenten:** Sollen reine Datenhalter (POD - Plain Old Data) sein (`src/Spark/Renderer/Components.h`).
    - **Systeme:** Die Logik wird in den `OnUpdate`-Methoden der Layer oder in Lua-Skripten verarbeitet.
- **Entkoppelung von Systemen:** Globale Dienste wie `AudioManager`, `AssetManager` oder `ScriptEngine` sollten über statische Schnittstellen oder Singletons zugänglich sein, aber keine harten Abhängigkeiten zu spezifischen Szenen-Objekten haben.
- **Batch Rendering:** Der `Renderer2D` ist auf Batching ausgelegt. Vermeide es, `glDraw...` direkt aufzurufen. Nutze immer die `Renderer2D::Draw...` Funktionen.

## 3. Ressourcen-Management (Assets)
- **AssetHandles:** Lade Assets (Texturen, Sounds) NIEMALS direkt über Pfade in Komponenten. Nutze immer den `AssetManager` und speichere nur den `AssetHandle`.
- **Lebenszyklus:** Achte darauf, dass beim Zerstören von Entitäten oder beim Szenenwechsel auch die zugehörigen Laufzeit-Ressourcen (z.B. Box2D-Bodies, ma_sound-Pointer) sauber freigegeben werden (siehe `DestroyEntity`).

## 4. Fehlerbehebung & Validierung
- Bei Abstürzen (Signal 6/Abort) immer zuerst auf Null-Pointer bei Komponenten-Zugriffen prüfen.
- Immer `HasComponent<T>()` prüfen, bevor `GetComponent<T>()` aufgerufen wird (siehe `src/Spark/Renderer/Entity.h`).

## 4. Dateistruktur
- `src/Spark/Core`: Kernsysteme (Log, Window, Input).
- `src/Spark/Renderer`: Grafik, Szene, ECS-Komponenten.
- `src/Spark/Editor`: ImGui-Panels und Editor-Logik.
- `libs/`: Externe Abhängigkeiten.
