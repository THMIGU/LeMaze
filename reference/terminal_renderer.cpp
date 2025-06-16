/********************************************
 *  Project     : Terminal Renderer
 *  File        : terminal_renderer.cpp
 *  Author      : Kai Parsons
 *  Date        : 2025-06-07
 *  Description : Uses a matrix of pixels and
 *                renders them to the
 *                terminal.
 ********************************************/

#include <iostream>
#include <unordered_map>
#include <windows.h>
#include <vector>

using namespace std;

const unordered_map<string, string> color_codes = {
        {"fg_black",   "\033[30m"},
        {"fg_red",     "\033[31m"},
        {"fg_green",   "\033[32m"},
        {"fg_yellow",  "\033[33m"},
        {"fg_blue",    "\033[34m"},
        {"fg_magenta", "\033[35m"},
        {"fg_cyan",    "\033[36m"},
        {"fg_white",   "\033[37m"},
        {"bg_black",   "\033[40m"},
        {"bg_red",     "\033[41m"},
        {"bg_green",   "\033[42m"},
        {"bg_yellow",  "\033[43m"},
        {"bg_blue",    "\033[44m"},
        {"bg_magenta", "\033[45m"},
        {"bg_cyan",    "\033[46m"},
        {"bg_white",   "\033[47m"},
        {"reset",      "\033[0m"}
};

namespace terminal {
    void enable_virtual_terminal() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void init() {
        enable_virtual_terminal();
        SetConsoleOutputCP(CP_UTF8);
    }
}

void render(const vector<vector<string>>& matrix) {
    for (size_t row = 0; row + 1 < matrix.size(); row += 2) {
        for (size_t col = 0; col < matrix[row].size(); ++col) {
            const string& top = matrix[row][col];
            const string& bottom = matrix[row + 1][col];

            string fg_code = color_codes.at("fg_" + top);
            string bg_code = color_codes.at("bg_" + bottom);

            cout << fg_code << bg_code << "▀";
        }

        cout << color_codes.at("reset") << endl;
    }

    size_t last_row = matrix.size() - 1;

    if (matrix.size() % 2 != 0) {
        for (const auto & px : matrix[last_row]) {
            string code = color_codes.at("fg_" + px);
            cout << code << "▀";
        }

        cout << color_codes.at("reset") << endl;
    }
}

int main() {
    terminal::init();

    vector<vector<string>> matrix = {
            {"red", "cyan", "red"},
            {"cyan", "red", "cyan"},
            {"red", "cyan", "red"}
    };

    render(matrix);

    return 0;
}