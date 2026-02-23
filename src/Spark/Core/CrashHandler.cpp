#include "CrashHandler.h"
#include <iostream>
#include <signal.h>
#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>

namespace Spark {

    void CrashHandler::Init() {
        signal(SIGSEGV, HandleCrash);
        signal(SIGABRT, HandleCrash);
        signal(SIGILL,  HandleCrash);
        signal(SIGFPE,  HandleCrash);
    }

    void CrashHandler::HandleCrash(int signal) {
        std::cerr << "CRITICAL ERROR: Spark Engine received signal " << signal << " (" << GetSignalName(signal) << ")" << std::endl;
        
        // Show native alert on macOS
        std::string alertCmd = "osascript -e 'display dialog \"Spark Engine has crashed!\\n\\nSignal: " + 
                               std::to_string(signal) + " (" + GetSignalName(signal) + 
                               ")\\n\\nA diagnostic crash report has been created in the crashes/ folder.\" " +
                               "with title \"Engine Crash\" buttons {\"OK\"} default button \"OK\" with icon stop'";
        std::system(alertCmd.c_str());

        CreateCrashReport(signal);
        
        // Programm nach Report beenden
        exit(signal);
    }

    void CrashHandler::CreateCrashReport(int signal) {
        try {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
            
            std::string crashDir = "crashes/crash_" + ss.str();
            std::filesystem::create_directories(crashDir);

            // 1. Crash Info Datei schreiben
            std::ofstream info(crashDir + "/report.txt");
            info << "Spark Engine Crash Report" << std::endl;
            info << "=========================" << std::endl;
            info << "Signal: " << signal << " (" << GetSignalName(signal) << ")" << std::endl;
            info << "Time: " << ss.str() << std::endl;
            info.close();

            // 2. Wichtige Dateien kopieren
            CopyFileSafe("assets/scenes/Example.scene", crashDir + "/Example.scene");
            CopyFileSafe("assets/plan.yaml", crashDir + "/plan.yaml");
            CopyFileSafe("Spark.log", crashDir + "/Spark.log");
            CopyFileSafe("assets/AssetRegistry.yaml", crashDir + "/AssetRegistry.yaml");

            std::cerr << "CRASH REPORT CREATED AT: " << crashDir << std::endl;
            std::cerr << "Please provide this folder to the developers for debugging." << std::endl;

        } catch (...) {
            std::cerr << "FAILED TO CREATE CRASH REPORT!" << std::endl;
        }
    }

    void CrashHandler::CopyFileSafe(const std::filesystem::path& from, const std::filesystem::path& to) {
        if (std::filesystem::exists(from)) {
            std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
        }
    }

    std::string CrashHandler::GetSignalName(int signal) {
        switch (signal) {
            case SIGSEGV: return "Segmentation Fault (Memory access error)";
            case SIGABRT: return "Abort (Assertion failed or internal error)";
            case SIGILL:  return "Illegal Instruction";
            case SIGFPE:  return "Floating Point Exception";
        }
        return "Unknown Signal";
    }

}
