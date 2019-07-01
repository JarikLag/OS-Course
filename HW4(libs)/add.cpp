//
// Created by yaroslav on 24.04.19.
//
#include <iostream>
#include "add.h"

int add(int a, int b) {
    std::cout << "It is add function from static library" << std::endl;
    return a + b;
}