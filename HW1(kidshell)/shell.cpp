#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>

int main() {
    std::cout << "~> ";
    std::string command;
    while (getline(std::cin, command)) {
        if (command == "exit") {
            exit(EXIT_SUCCESS);
        }
        // Parse command
        std::istringstream inputStream(command);
        std::vector<std::string> arguments;
        std::string argument;
        while (inputStream >> argument) {
            arguments.emplace_back(std::move(argument));
        }
        // Fork
        pid_t pid = fork();
        int status;
        if (pid == -1) {
            std::cerr << "Can't fork process: " << strerror(errno) << std::endl;
            continue;
        }
        else if (pid == 0) {
            // We are child
            // Convert vector of arguments into char**
            size_t size = arguments.size();
            char** convertedArguments = new char* [size + 1];
            for (size_t i = 0; i < size; ++i) {
                convertedArguments[i] = const_cast<char*>(arguments[i].data());
            }
            convertedArguments[size] = nullptr;
            // Start execution
            char* env[] = {nullptr};
            status = execve(convertedArguments[0], convertedArguments, env);
            if (status == -1) {
                std::cerr << "Execution failed: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0) {
            // We are parent
            if (waitpid(pid, &status, 0) == -1) {
                std::cerr << "Failed to wait for child return: " << strerror(errno) << std::endl;
            } else {
                std::cout << "Program returned with code: " << WEXITSTATUS(status) << std::endl;
            }
        }
        std::cout << "~> ";
    }
    return 0;
}
