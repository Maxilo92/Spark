#include "CrashHandler.h"
#include "Log.h"
#include <iostream>
#include <signal.h>
#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Spark {

    void CrashHandler::Init() {
        struct sigaction action;
        action.sa_handler = HandleCrash;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        sigaction(SIGSEGV, &action, nullptr);
        sigaction(SIGABRT, &action, nullptr);
        sigaction(SIGILL,  &action, nullptr);
        sigaction(SIGFPE,  &action, nullptr);
        sigaction(SIGBUS,  &action, nullptr);
    }

    void CrashHandler::HandleCrash(int signal) {
        // We are in a very sensitive state here. 
        // Avoid complex logic if possible, but we need the report.
        
        // Disable further signal handling to avoid recursion
        struct sigaction action;
        action.sa_handler = SIG_DFL;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGSEGV, &action, nullptr);
        sigaction(SIGABRT, &action, nullptr);

        std::cerr << "\n[CRITICAL] Spark Engine Crashed!" << std::endl;
        std::cerr << "Signal: " << signal << " (" << GetSignalName(signal) << ")" << std::endl;

        CreateCrashReport(signal);
        
        std::cerr << "Engine shutting down safely..." << std::endl;
        
        // Use _exit to avoid calling atexit handlers which might crash again
        _exit(signal);
    }

    void CrashHandler::CreateCrashReport(int signal) {
        // Attempt to write a report even though it's technically unsafe
        try {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
            
            std::string crashDir = "crashes/crash_" + ss.str();
            
            // Standard C mkdir might be safer than std::filesystem in a crash
            #ifdef _WIN32
            _mkdir("crashes");
            _mkdir(crashDir.c_str());
            #else
            mkdir("crashes", 0777);
            mkdir(crashDir.c_str(), 0777);
            #endif

            // 1. Crash Info
            std::ofstream info(crashDir + "/report.txt");
            if (info.is_open()) {
                info << "Spark Engine Crash Report\n";
                info << "=========================\n";
                info << "Time:   " << ss.str() << "\n";
                info << "Signal: " << signal << " (" << GetSignalName(signal) << ")\n";
                info << "PID:    " << getpid() << "\n";
                info << "\n--- Engine State ---\n";
                info << "Status: CRASHED DURING RUNTIME\n";
                info.close();
            }

            // 2. Log
            std::ofstream logFile(crashDir + "/Spark.log");
            if (logFile.is_open()) {
                logFile << "--- ENGINE LOG AT TIME OF CRASH ---\n";
                logFile << Log::GetFullLogString();
                logFile.close();
            }

            // 3. Important Files
            CopyFileSafe("assets/scenes/Example.scene", crashDir + "/Example.scene");
            CopyFileSafe("assets/scenes/Adventure.scene", crashDir + "/Adventure.scene");
            CopyFileSafe("assets/plan.yaml", crashDir + "/plan.yaml");
            CopyFileSafe("assets/AssetRegistry.yaml", crashDir + "/AssetRegistry.yaml");

            std::cerr << "Crash report saved to: " << crashDir << std::endl;

        } catch (...) {
            // If we crash here, there's nothing more we can do
        }
    }

    void CrashHandler::CopyFileSafe(const std::filesystem::path& from, const std::filesystem::path& to) {
        if (std::filesystem::exists(from)) {
            std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
        }
    }

    std::string CrashHandler::GetSignalName(int signal) {
        switch (signal) {
            case SIGSEGV: return "Segmentation Fault (SIGSEGV)";
            case SIGABRT: return "Abort (SIGABRT)";
            case SIGILL:  return "Illegal Instruction (SIGILL)";
            case SIGFPE:  return "Floating Point Exception (SIGFPE)";
            case SIGBUS:  return "Bus Error (SIGBUS)";
        }
        return "Unknown Signal " + std::to_string(signal);
    }

}
