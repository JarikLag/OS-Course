//
// Created by yaroslav on 24.04.19.
//

#include <iostream>
#include "mod.h"

extern "C"
int mod(int a, int b) {
    std::cout << "It is mod function from loaded library" << std::endl;
    return a % b;
}