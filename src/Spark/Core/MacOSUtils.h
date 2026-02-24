#pragma once

#include <string>

namespace Spark {
    class MacOSUtils {
    public:
        // Opens a native macOS file dialog and returns the selected path
        static std::string OpenFile(const char* filter);

        // Überschreibt den Standard "About" Befehl im macOS App-Menü
        static void SetupApplicationMenu();
        static bool ShouldShowAbout();
        static void ResetAboutFlag();
    };
}
