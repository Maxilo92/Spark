#include "InstanceLock.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/errno.h>

namespace Spark {

    static std::string s_LockFileName;

    std::filesystem::path InstanceLock::GetLockFilePath(const std::string& name) {
        return std::filesystem::current_path() / name;
    }

    void InstanceLock::KillProcess(int pid) {
        if (pid <= 0) return;
        
        // Versuche, den Prozess höflich zu beenden
        kill(pid, SIGTERM);
        
        // Kurze Wartezeit (simuliert), um dem Prozess Zeit zum Aufräumen zu geben
        // In einer echten App könnte man hier pollen, ob der Prozess weg ist
    }

    bool InstanceLock::IsProcessRunning(int pid) {
        // Signal 0 sendet kein Signal, prüft aber, ob der Prozess existiert
        return kill(pid, 0) == 0;
    }

    bool InstanceLock::Lock(const std::string& lockFileName) {
        s_LockFileName = lockFileName;
        auto path = GetLockFilePath(lockFileName);

        if (std::filesystem::exists(path)) {
            std::ifstream file(path);
            int oldPid;
            if (file >> oldPid) {
                if (IsProcessRunning(oldPid)) {
                    std::cout << "Found existing instance (PID " << oldPid << "). Terminating..." << std::endl;
                    KillProcess(oldPid);
                    // Wir geben dem OS kurz Zeit, den Prozess zu cleanen
                    sleep(1); 
                } else {
                    std::cout << "Found stale lock file. Overwriting." << std::endl;
                }
            }
            file.close();
        }

        // Neue Lock-Datei schreiben
        std::ofstream lockFile(path);
        if (lockFile.is_open()) {
            lockFile << getpid();
            lockFile.flush();
            return true;
        }

        std::cerr << "Failed to create lock file!" << std::endl;
        return false;
    }

    void InstanceLock::Unlock() {
        auto path = GetLockFilePath(s_LockFileName);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    }

}
