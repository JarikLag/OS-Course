#include <iostream>
#include <dlfcn.h>
#include "add.h"
#include "mul.h"

int main() {
    std::cout << "Program prints sum, product and modulo of two numbers." << std::endl;
    std::cout << "Enter two numbers: " << std::endl;
    int a, b;
    std::cin >> a >> b;
    std::cout << add(a, b) << std::endl;
    std::cout << mul(a, b) << std::endl;
    void *loadedLib = dlopen("./libMod.so", RTLD_LAZY);
    if (loadedLib == nullptr) {
        std::cout << "Can't load external lib: " << dlerror() << std::endl;
        exit(EXIT_FAILURE);
    }
    auto mod = (int (*)(int, int))dlsym(loadedLib, "mod");
    if (mod == nullptr) {
        std::cout << "Can't load external function: " << dlerror() << std::endl;
        dlclose(loadedLib);
        exit(EXIT_FAILURE);
    }
    std::cout << mod(a, b) << std::endl;
    dlclose(loadedLib);
    return 0;
}