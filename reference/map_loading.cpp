#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <stack>
#include <windows.h>
#include <vector>
#include <conio.h>
#include <cmath>
#include <algorithm>
#include "resources/resources.h"

using namespace std;

namespace terminal {
    void enable_virtual_terminal() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void hide_cursor() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hOut, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hOut, &cursorInfo);
    }

    void init() {
        enable_virtual_terminal();
        hide_cursor();
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleTitle("Le Maze");
    }
}

namespace render {
    const unordered_map<string, string> color_codes = {
            {"fg_0",  "\033[30m"},
            {"fg_1",  "\033[37m"},
            {"fg_2",  "\033[31m"},
            {"fg_3",  "\033[33m"},
            {"bg_0",  "\033[40m"},
            {"bg_1",  "\033[47m"},
            {"bg_2",  "\033[41m"},
            {"bg_3",  "\033[43m"},
            {"reset", "\033[0m"}
    };

    constexpr string PIXEL = "▀";

    void up(int characters, ostringstream& out) {
        out << "\033[" << characters << "A";
    }

    void right(int characters, ostringstream& out) {
        out << "\033[" << characters << "C";
    }

    void render_pixel(ostringstream& out, const vector<vector<char>>& matrix, size_t row, size_t col) {
        const char& top = matrix[row][col];
        const char& bottom = matrix[row + 1][col];

        string fg_code = color_codes.at("fg_" + string(1, top));
        string bg_code = color_codes.at("bg_" + string(1, bottom));

        out << fg_code << bg_code << PIXEL;
    }

    void render(vector<vector<char>>& old_matrix, const vector<vector<char>>& new_matrix, bool first_frame) {
        ostringstream out;

        if (!first_frame) up(static_cast<int>(ceil(old_matrix.size() / 2) + 1), out);

        for (size_t row = 0; row + 1 < old_matrix.size(); row += 2) {
            for (size_t col = 0; col < old_matrix[row].size(); ++col) {
                if (!first_frame) {
                    if (
                            old_matrix[row][col] != new_matrix[row][col] ||
                            old_matrix[row + 1][col] != new_matrix[row + 1][col]
                            ) {
                        render_pixel(out, new_matrix, row, col);
                    } else right(1, out);
                } else render_pixel(out, new_matrix, row, col);
            }

            out << color_codes.at("reset") << endl;
        }

        if (old_matrix.size() % 2 != 0) {
            size_t last_row = old_matrix.size() - 1;

            for (int i = 0; i <= old_matrix[last_row].size() - 1; ++i) {
                if (!first_frame) {
                    if (old_matrix[last_row][i] != new_matrix[last_row][i]) {
                        string code = color_codes.at("fg_" + string(1, new_matrix[last_row][i]));
                        out << code << PIXEL;
                    } else right(1, out);
                } else {
                    string code = color_codes.at("fg_" + string(1, new_matrix[last_row][i]));
                    out << code << PIXEL;
                }
            }

            out << color_codes.at("reset") << endl;
        }

        cout << out.str();
    }
}

namespace game {
    const unordered_set<char> solids = {'1', '2', '3'};
    constexpr char PLAYER_PX = '2';

    enum Key {
        ArrowPrefix = 224,
        CtrlC = 3,
        Up = 72,
        Down = 80,
        Left = 75,
        Right = 77
    };

    const vector<int> EXIT_CODE = {2, 2};

    vector<vector<char>> update_matrix(
            const vector<vector<char>>& map,
            vector<vector<char>>& old_matrix,
            pair<int, int>& player_location,
            const vector<int>& offset
    ) {
        int x = player_location.first;
        int y = player_location.second;

        int new_x = x + offset[0];
        int new_y = y + offset[1];

        if (new_x < 0 || new_y < 0) return old_matrix;
        if (new_x > old_matrix.size() - 1 || new_y > old_matrix[0].size() - 1) return old_matrix;

        if (!solids.contains(old_matrix[new_x][new_y])) {
            vector<vector<char>> new_matrix = old_matrix;

            new_matrix[x][y] = map[x][y];
            new_matrix[new_x][new_y] = PLAYER_PX;

            player_location = {new_x, new_y};

            return new_matrix;
        }

        return old_matrix;
    }

    vector<int> input() {
        int ch = _getch();
        if (ch == ArrowPrefix) {
            vector<int> offset = {0, 0};

            switch (_getch()) {
                case Up:
                    offset[0] -= 1;
                    break;
                case Down:
                    offset[0] += 1;
                    break;
                case Left:
                    offset[1] -= 1;
                    break;
                case Right:
                    offset[1] += 1;
                    break;
            }

            return offset;
        } else if (ch == CtrlC) {
            return EXIT_CODE;
        }

        return {0, 0};
    }
}

namespace map {
    const vector<pair<int, int>> directions = {
            {0, 2},
            {0, -2},
            {2, 0},
            {-2, 0}
    };

    void dfs(int start_r, int start_c, vector<vector<char>>& maze, unordered_set<string>& visited) {
        stack<tuple<int, int, vector<pair<int, int>>>> stk;
        stk.emplace(start_r, start_c, directions);
        visited.insert(to_string(start_r) + "," + to_string(start_c));

        random_device rd;
        mt19937 gen(rd());

        while (!stk.empty()) {
            auto& [r, c, dirs] = stk.top();

            if (dirs.empty()) {
                stk.pop();
                continue;
            }

            auto dir = dirs.back();
            dirs.pop_back();

            int dr = dir.first, dc = dir.second;
            int nr = r + dr;
            int nc = c + dc;

            if (nr <= 0 || nc <= 0 || nr >= maze.size() - 1 || nc >= maze[0].size() - 1)
                continue;

            string key = to_string(nr) + "," + to_string(nc);
            if (visited.count(key)) continue;

            maze[r + dr / 2][c + dc / 2] = '0';
            visited.insert(key);

            vector<pair<int, int>> new_dirs = directions;
            shuffle(new_dirs.begin(), new_dirs.end(), gen);
            stk.emplace(nr, nc, new_dirs);
        }
    }
}

int main() {
    terminal::init();

    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(IDR_TXT1), RT_RCDATA);
    HGLOBAL hData = LoadResource(hModule, hRes);
    const char* pData = static_cast<const char*>(LockResource(hData));
    DWORD size = SizeofResource(hModule, hRes);
    string text(pData, size);

    vector<vector<char>> map(1);

    for (const auto& character : text) {
        if (character == '\r') {
            continue;
        } else if (character == '\n') {
            map.emplace_back();
        } else {
            map.back().push_back(character);
        }
    }

    unordered_set<string> visited;
    map::dfs(1, 1, map, visited);

    vector<vector<char>> old_matrix = map;

    pair<int, int> player_location = {1, 1};
    old_matrix[player_location.first][player_location.second] = '2';

    render::render(old_matrix, old_matrix, true);

    vector<vector<char>> new_matrix;
    vector<int> offset;

    while (true) {
        offset = game::input();
        new_matrix = game::update_matrix(map, old_matrix, player_location, offset);

        if (offset == vector<int>{2, 2}) break;

        if (new_matrix != old_matrix) {
            render::render(old_matrix, new_matrix, false);
            old_matrix = new_matrix;
        }
    }

    return 0;
}
