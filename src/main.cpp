#include "Application.h"
#include "Log.h"
#include <string>
#include <vector>

int main(int argc, char** argv) {
    bool verbose = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        }
    }

    if (verbose) {
        Spark::Log::SetVerbose(true);
    }

    Application app;
    app.Run();
    return 0;
}
