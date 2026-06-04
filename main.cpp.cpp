REPL loop:

#include <iostream>
#include <string>
#include <algorithm>

int main() {
    std::string input;

    while (true) {
        std::cout << "db > ";
        std::getline(std::cin, input);

        // Remove leading and trailing spaces
        size_t start = input.find_first_not_of(" \t");
        size_t end = input.find_last_not_of(" \t");

        if (start == std::string::npos) {
            // Input was empty or just spaces
            continue;
        }

        input = input.substr(start, end - start + 1);

        if (input == ".exit") {
            std::cout << "Goodbye!\n";
            break;
        } else {
            std::cout << "Unknown command: '" << input << "'\n";
        }
    }

    return 0;
}