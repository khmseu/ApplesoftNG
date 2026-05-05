#include "platform/console.hpp"
#include <iostream>

void print(const std::string& message) {
    std::cout << message << std::endl;
}

std::string readLine() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}