#pragma once

namespace Spark {
    class MacOSUtils {
    public:
        // Überschreibt den Standard "About" Befehl im macOS App-Menü
        static void SetupApplicationMenu();
        static bool ShouldShowAbout();
        static void ResetAboutFlag();
    };
}
