#include "Log.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

namespace Spark {

    std::vector<LogMessage> Log::s_Messages;
    std::ofstream Log::s_LogFile;
    std::recursive_mutex Log::s_Mutex;
    std::map<std::string, Log::CommandCallback> Log::s_Commands;

    void Log::Init() {
        s_LogFile.open("spark.log", std::ios::out | std::ios::trunc);
        
        // Standard-Befehle registrieren
        RegisterCommand("clear", [](const std::vector<std::string>& args) {
            Log::Clear();
        });
        
        RegisterCommand("help", [](const std::vector<std::string>& args) {
            SP_INFO("Available commands:");
            for (auto const& [name, cb] : s_Commands)
                SP_INFO(" - " + name);
        });

        SP_INFO("Spark Engine Logger Initialized.");
    }

    void Log::Shutdown() {
        if (s_LogFile.is_open()) {
            s_LogFile.close();
        }
    }

    void Log::RegisterCommand(const std::string& command, const CommandCallback& callback) {
        std::lock_guard<std::recursive_mutex> lock(s_Mutex);
        s_Commands[command] = callback;
    }

    void Log::ExecuteCommand(const std::string& commandLine) {
        if (commandLine.empty()) return;

        SP_TRACE("> " + commandLine);

        std::stringstream ss(commandLine);
        std::string cmd;
        ss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        std::vector<std::string> args;
        std::string arg;
        while (ss >> arg) args.push_back(arg);

        std::lock_guard<std::recursive_mutex> lock(s_Mutex);
        if (s_Commands.find(cmd) != s_Commands.end()) {
            s_Commands[cmd](args);
        } else {
            SP_ERROR("Unknown command: " + cmd);
        }
    }

    void Log::AddMessage(LogLevel level, const std::string& msg) {
        std::lock_guard<std::recursive_mutex> lock(s_Mutex);
        
        std::string ts = GetTimestamp();
        s_Messages.push_back({ msg, level, ts });

        if (s_LogFile.is_open()) {
            s_LogFile << "[" << ts << "] " << msg << std::endl;
            s_LogFile.flush();
        }

        std::cout << "[" << ts << "] " << msg << std::endl;
    }

    std::string Log::GetTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

    void Log::Trace(const std::string& msg) { AddMessage(LogLevel::Trace, msg); }
    void Log::Info(const std::string& msg) { AddMessage(LogLevel::Info, msg); }
    void Log::Warn(const std::string& msg) { AddMessage(LogLevel::Warning, msg); }
    void Log::Error(const std::string& msg) { AddMessage(LogLevel::Error, msg); }
    void Log::Critical(const std::string& msg) { AddMessage(LogLevel::Critical, msg); }

    void Log::Clear() {
        std::lock_guard<std::recursive_mutex> lock(s_Mutex);
        s_Messages.clear();
    }

}
