#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <functional>
#include <map>

namespace Spark {

    enum class LogLevel {
        Trace = 0,
        Info,
        Warning,
        Error,
        Critical
    };

    struct LogMessage {
        std::string Message;
        LogLevel Level;
        std::string Timestamp;
    };

    class Log {
    public:
        using CommandCallback = std::function<void(const std::vector<std::string>&)>;

        static void Init();
        static void Shutdown();

        static void RegisterCommand(const std::string& command, const CommandCallback& callback);
        static void ExecuteCommand(const std::string& commandLine);

        static void Trace(const std::string& msg);
        static void Info(const std::string& msg);
        static void Warn(const std::string& msg);
        static void Error(const std::string& msg);
        static void Critical(const std::string& msg);

        static void Clear();

        static std::vector<LogMessage> GetMessages();
        static std::string GetFullLogString();
        
        static void SetVerbose(bool enabled) { s_VerboseLogging = enabled; }
        static bool IsVerbose() { return s_VerboseLogging; }

    private:
        static void AddMessage(LogLevel level, const std::string& msg);
        static std::string GetTimestamp();

    private:
        static std::vector<LogMessage> s_Messages;
        static std::ofstream s_LogFile;
        static std::recursive_mutex s_Mutex;
        static std::map<std::string, CommandCallback> s_Commands;
        static bool s_VerboseLogging;
    };

}

// Macros für einfachere Nutzung
#define SP_TRACE(...)    ::Spark::Log::Trace(__VA_ARGS__)
#define SP_INFO(...)     ::Spark::Log::Info(__VA_ARGS__)
#define SP_WARN(...)     ::Spark::Log::Warn(__VA_ARGS__)
#define SP_ERROR(...)    ::Spark::Log::Error(__VA_ARGS__)
#define SP_CRITICAL(...) ::Spark::Log::Critical(__VA_ARGS__)

// Verbose logging that can be toggled
#define SP_DEBUG_TRACE(...) if (::Spark::Log::IsVerbose()) ::Spark::Log::Trace(__VA_ARGS__)
