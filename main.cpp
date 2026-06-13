#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>

const int MAX_ROWS = 100;
const int PAGE_SIZE = 4096;
const int MAX_PAGES = 100;
const int ROW_SIZE = sizeof(int) + 50 + 255;
const int ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;

struct Row {
    int id;
    std::string username;
    std::string email;
};

void printRow(const Row& row) {
    std::cout << "(" << row.id << ", " << row.username << ", " << row.email << ")\n";
}

void serializeRow(const Row& row, void* dest) {
    char* ptr = (char*)dest;
    memcpy(ptr, &row.id, sizeof(int));
    ptr += sizeof(int);

    char username[50] = {};
    char email[255] = {};
    strncpy(username, row.username.c_str(), 49);
    strncpy(email, row.email.c_str(), 254);

    memcpy(ptr, username, 50);
    ptr += 50;
    memcpy(ptr, email, 255);
}

void deserializeRow(const void* src, Row& row) {
    const char* ptr = (const char*)src;
    memcpy(&row.id, ptr, sizeof(int));
    ptr += sizeof(int);

    char username[50] = {};
    char email[255] = {};
    memcpy(username, ptr, 50);
    ptr += 50;
    memcpy(email, ptr, 255);

    row.username = std::string(username);
    row.email = std::string(email);
}

class Pager {
public:
    std::fstream file;
    void* pages[MAX_PAGES];
    int numPages;

    Pager(const std::string& filename) {
        for (int i = 0; i < MAX_PAGES; i++) pages[i] = nullptr;
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }
        file.seekg(0, std::ios::end);
        int fileSize = (int)file.tellg();
        numPages = fileSize / PAGE_SIZE;
    }

    void* getPage(int pageNum) {
        if (pages[pageNum] == nullptr) {
            pages[pageNum] = malloc(PAGE_SIZE);
            memset(pages[pageNum], 0, PAGE_SIZE);
            if (pageNum < numPages) {
                file.seekg(pageNum * PAGE_SIZE);
                file.read((char*)pages[pageNum], PAGE_SIZE);
            }
        }
        return pages[pageNum];
    }

    void flushAll(int numRows) {
        int fullPages = numRows / ROWS_PER_PAGE;
        for (int i = 0; i < fullPages; i++) {
            if (pages[i]) {
                file.seekp(i * PAGE_SIZE);
                file.write((char*)pages[i], PAGE_SIZE);
            }
        }
        int remainingRows = numRows % ROWS_PER_PAGE;
        if (remainingRows > 0 && pages[fullPages]) {
            file.seekp(fullPages * PAGE_SIZE);
            file.write((char*)pages[fullPages], remainingRows * ROW_SIZE);
        }
        file.flush();
    }

    ~Pager() {
        file.close();
        for (int i = 0; i < MAX_PAGES; i++) {
            if (pages[i]) free(pages[i]);
        }
    }
};

struct Table {
    Pager* pager;
    int numRows;

    Table(const std::string& filename) {
        pager = new Pager(filename);
        std::ifstream f(filename, std::ios::binary | std::ios::ate);
        int fileSize = (int)f.tellg();
        numRows = fileSize / ROW_SIZE;
    }

    void* rowSlot(int rowNum) {
        int pageNum = rowNum / ROWS_PER_PAGE;
        void* page = pager->getPage(pageNum);
        int rowOffset = rowNum % ROWS_PER_PAGE;
        return (char*)page + (rowOffset * ROW_SIZE);
    }

    ~Table() {
        pager->flushAll(numRows);
        delete pager;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: mydb <database file>\n";
        return 1;
    }
    std::string filename = argv[1];
    Table table(filename);
    std::string input;

    while (true) {
        std::cout << "db > ";
        std::getline(std::cin, input);
        size_t start = input.find_first_not_of(" \t");
        size_t end = input.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        input = input.substr(start, end - start + 1);

        if (input == ".exit") {
            std::cout << "Goodbye!\n";
            break;
        } else if (input == "select") {
            if (table.numRows == 0) {
                std::cout << "No rows found.\n";
            } else {
                for (int i = 0; i < table.numRows; i++) {
                    Row row;
                    deserializeRow(table.rowSlot(i), row);
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
            } else if (table.numRows >= MAX_ROWS) {
                std::cout << "Error: table is full.\n";
            } else {
                bool duplicate = false;
                for (int i = 0; i < table.numRows; i++) {
                    Row r;
                    deserializeRow(table.rowSlot(i), r);
                    if (r.id == row.id) { duplicate = true; break; }
                }
                if (duplicate) {
                    std::cout << "Error: duplicate ID " << row.id << "\n";
                } else {
                    serializeRow(row, table.rowSlot(table.numRows));
                    table.numRows++;
                    std::cout << "Row inserted.\n";
                }
            }
        } else {
            std::cout << "Unknown command: '" << input << "'\n";
        }
    }
    return 0;
}