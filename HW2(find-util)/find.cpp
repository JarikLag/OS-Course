#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

typedef long long LL;

std::vector<std::string> files;
bool isInumEnabled, isNameEnabled, isSizeEnabled, isNlinksEnabled, isExecEnabled;
ino_t inodeNum;
int sizeType;
off_t sizeNum;
nlink_t nlinkNum;
std::string fileName, execPath, startPath;

LL parseNumber(const std::string& str) {
    LL result = 0;
    try {
        result = std::stoll(str);
    } catch (const std::exception& ex) {
        std::cerr << "Invalid arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    return result;
}

bool checkFile(const std::string& path, const std::string& name) {
    struct stat info{};
    if (stat(path.data(), &info) == -1) {
        std::cerr << "Can't read file statistics " << path << ": " << strerror(errno) << std::endl;
        return false;
    }
    else {
        if (isInumEnabled && info.st_ino != inodeNum) {
           return false;
        }
        if (isNameEnabled && name != fileName) {
            return false;
        }
        if (isSizeEnabled) {
            if (sizeType == -1 && info.st_size >= sizeNum) {
                return false;
            }
            if (sizeType == 0 && info.st_size != sizeNum) {
                return false;
            }
            if (sizeType == 1 && info.st_size <= sizeNum) {
                return false;
            }
        }
        if (isNlinksEnabled && info.st_nlink != nlinkNum) {
            return false;
        }
        return true;
    }
}

void dfs(const std::string& directoryName) {
    DIR* directory = opendir(directoryName.data());
    if (directory == nullptr) {
        std::cerr << "Can't open directory " << directoryName << ": " << strerror(errno) << std::endl;
        return;
    }
    while (true) {
        dirent* entry = readdir(directory);
        if (entry == nullptr) {
            break;
        }
        std::string currentEntryName(entry->d_name);
        if (currentEntryName == "." || currentEntryName == "..") {
            continue;
        }
        std::string nextPath(directoryName);
        // If we start from directory, which already ends with (/), output will be correct, but weird
        if (!directoryName.empty() && directoryName.back() != '/') {
            nextPath.append("/");
        }
        nextPath.append(currentEntryName);
        if (entry->d_type == DT_DIR) {
            dfs(nextPath);
        }
        else if (entry->d_type == DT_REG) {
            if (checkFile(nextPath, currentEntryName)) {
                files.emplace_back(nextPath);
            }
        }
    }
    closedir(directory);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc % 2 == 1) {
        std::cerr << "Not enough arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<std::string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.emplace_back(argv[i]);
    }
    startPath = arguments[0];
    for (size_t i = 1; i < arguments.size(); ++i) {
        if (arguments[i] == "-inum") {
            inodeNum = static_cast<ino_t>(parseNumber(arguments[++i]));
            isInumEnabled = true;
        }
        else if (arguments[i] == "-name") {
            fileName = arguments[++i];
            isNameEnabled = true;
        }
        else if (arguments[i] == "-size") {
            if (arguments[i + 1][0] == '-') {
                sizeType = -1;
            }
            else if (arguments[i + 1][0] == '=') {
                sizeType = 0;
            }
            else if (arguments[i + 1][0] == '+') {
                sizeType = 1;
            }
            else {
                std::cerr << "Invalid arguments" << std::endl;
                exit(EXIT_FAILURE);
            }
            sizeNum = static_cast<off_t>(parseNumber(arguments[++i].substr(1)));
            isSizeEnabled = true;
        }
        else if (arguments[i] == "-nlinks") {
            nlinkNum = static_cast<nlink_t>(parseNumber(arguments[++i]));
            isNlinksEnabled = true;
        }
        else if (arguments[i] == "-exec") {
            execPath = arguments[++i];
            isExecEnabled = true;
        }
        else {
            std::cerr << "Invalid arguments" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    dfs(startPath);
    if (isExecEnabled) {
        pid_t pid = fork();
        int status;
        if (pid == -1) {
            std::cerr << "Can't fork process: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            size_t size = files.size() + 1;
            char** convertedArguments = new char* [size + 1];
            convertedArguments[0] = const_cast<char*>(execPath.data());
            for (size_t i = 1; i <= size; ++i) {
                convertedArguments[i] = const_cast<char*>(files[i - 1].data());
            }
            convertedArguments[size] = nullptr;
            char* env[] = {nullptr};
            status = execve(convertedArguments[0], convertedArguments, env);
            if (status == -1) {
                std::cerr << "Execution failed: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0) {
            if (waitpid(pid, &status, 0) == -1) {
                std::cerr << "Failed to wait for child return: " << strerror(errno) << std::endl;
            } else {
                std::cout << "Program returned with code: " << WEXITSTATUS(status) << std::endl;
            }
        }
    }
    else {
        for (size_t i = 0; i < files.size(); ++i) {
            std::cout << files[i] << std::endl;
        }
    }
    return 0;
}
