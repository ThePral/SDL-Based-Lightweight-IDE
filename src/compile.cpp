#include "compile.hpp"

bool compile(const std::string &filename) {
    std::string compileCommand = "g++ " + filename + " -o output";
    int result = std::system(compileCommand.c_str());
    return (result == 0);
}

void run(const std::string &filename) {
    std::string runCommand = ".\\" + filename + ".exe ";
    std::system(runCommand.c_str());
}

void run_in_another_window(const std::string &filename) {
#ifdef _WIN32
    std::string runCommand = "start cmd /c \"" + filename + " & pause\"";
#elif __linux__
    std::string runCommand = "x-terminal-emulator -e \"" + filename + "; read -n 1 -s -r -p 'Press any key to continue...'\"";
#elif __APPLE__
    std::string runCommand = "osascript -e 'tell application \"Terminal\" to do script \"" + filename + "\"'";
#else
    std::cerr << "Unsupported OS!" << std::endl;
    return;
#endif
    std::system(runCommand.c_str());
}

void debug(const std::string &filename) {
    std::cout << "Debugging " << filename << " (simulated):\n";
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        int lineNumber = 1;
        while (std::getline(file, line)) {
            std::cout << lineNumber << ": " << line << std::endl;
            lineNumber++;
        }
        file.close();
    } else {
        std::cerr << "Unable to open file for debugging." << std::endl;
    }
}
