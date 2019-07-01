//
// Created by yaroslav on 24.04.19.
//

#include <iostream>
#include "mul.h"

int mul(int a, int b) {
    std::cout << "Is is mul function from dynamic library" << std::endl;
    return a * b;
}