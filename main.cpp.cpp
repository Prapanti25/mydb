#include <iostream>
#include <string>
#include <vector>
#include <sstream>

const int MAX_ROWS = 100;

struct Row {
    int id;
    std::string username;
    std::string email;
};

struct Table {
    std::vector<Row> rows;
};

void printRow(const Row& row) {
    std::cout << "(" << row.id << ", " << row.username << ", " << row.email << ")\n";
}

int main() {
    std::string input;
    Table table;

    while (true) {
        std::cout << "db > ";
        std::getline(std::cin, input);

        size_t start = input.find_first_not_of(" \t");
        size_t end = input.find_last_not_of(" \t");

        if (start == std::string::npos) {
            continue;
        }

        input = input.substr(start, end - start + 1);

        if (input == ".exit") {
            std::cout << "Goodbye!\n";
            break;
        } else if (input == "select") {
            if (table.rows.empty()) {
                std::cout << "No rows found.\n";
            } else {
                for (const Row& row : table.rows) {
                    printRow(row);
                }
            }
        } else if (input.substr(0, 6) == "insert") {
            std::istringstream iss(input);
            std::string command;
            Row row;

            iss >> command >> row.id >> row.username >> row.email;

            if (row.username.empty() || row.email.empty()) {
                std::cout << "Syntax error. Usage: insert <id> <username> <email>\n";
            } else if ((int)table.rows.size() >= MAX_ROWS) {
                std::cout << "Error: table is full.\n";
            } else {
                bool duplicate = false;
                for (const Row& r : table.rows) {
                    if (r.id == row.id) {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) {
                    std::cout << "Error: duplicate ID " << row.id << "\n";
                } else {
                    table.rows.push_back(row);
                    std::cout << "Row inserted.\n";
                }
            }
        } else {
            std::cout << "Unknown command: '" << input << "'\n";
        }
    }

    return 0;
}