/********************************************
 *  Project     : Le Maze
 *  File        : main.cpp
 *  Author      : Kai Parsons
 *  Date        : 2025-06-16
 *  Description : Simple maze game in C++
 ********************************************/

#include <iostream>
#include <sstream>
#include <conio.h>

#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <algorithm>
#include <cmath>
#include <chrono>
#include <random>

#include <windows.h>

using namespace std;

namespace terminal {
    void enable_virtual_terminal() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void cursor(bool visible) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hOut, &cursorInfo);

        if (visible) {
            cursorInfo.bVisible = TRUE;
        } else cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hOut, &cursorInfo);
    }

    void init() {
        enable_virtual_terminal();
        cursor(false);
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleTitle("Le Maze");
    }

    void deinit() {
        cursor(true);
    }
}

namespace map {
    const vector<pair<int, int>> directions = {
            {0, 2},
            {0, -2},
            {2, 0},
            {-2, 0}
    };

    constexpr char TILE_WALL = '1';
    constexpr char TILE_PATH = '0';
    constexpr char TILE_PLAYER = '2';
    constexpr char TILE_GOAL = '3';

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

            if (nr <= 0 || nc <= 0 || nr >= maze.size() - 1 || nc >= maze[0].size() - 1) continue;

            string key = to_string(nr) + "," + to_string(nc);
            if (visited.count(key)) continue;

            maze[r + dr / 2][c + dc / 2] = TILE_PATH;
            visited.insert(key);

            vector<pair<int, int>> new_dirs = directions;
            shuffle(new_dirs.begin(), new_dirs.end(), gen);
            stk.emplace(nr, nc, new_dirs);
        }
    }

    tuple<pair<int, int>, int> find_farthest_point(const vector<vector<char>>& maze, int start_r, int start_c) {
        size_t rows = maze.size(), cols = maze[0].size();
        vector<vector<bool>> visited(rows, vector<bool>(cols, false));
        queue<tuple<int, int, int>> q;

        q.emplace(start_r, start_c, 0);
        visited[start_r][start_c] = true;

        pair<int, int> farthest = {start_r, start_c};
        int max_dist = 0;

        vector<pair<int, int>> deltas = {
                {1, 0},
                {-1, 0},
                {0, 1},
                {0, -1}
        };

        while (!q.empty()) {
            auto [r, c, dist] = q.front();
            q.pop();

            if (dist > max_dist) {
                max_dist = dist;
                farthest = {r, c};
            }

            for (auto [dr, dc] : deltas) {
                int nr = r + dr, nc = c + dc;
                if (nr >= 0 && nc >= 0 && nr < rows && nc < cols &&
                    !visited[nr][nc] && maze[nr][nc] == TILE_PATH) {
                    visited[nr][nc] = true;
                    q.emplace(nr, nc, dist + 1);
                }
            }
        }

        return {farthest, max_dist};
    }

    vector<vector<char>> generate_empty_maze(unsigned int rows, unsigned int cols) {
        vector<vector<char>> maze(rows, vector<char>(cols, TILE_WALL));
        for (int r = 1; r < rows; r += 2) {
            for (int c = 1; c < cols; c += 2) {
                maze[r][c] = TILE_PATH;
            }
        }
        return maze;
    }

    tuple<vector<vector<char>>, pair<int, int>, int> generate_maze(
            unsigned int rows, unsigned int cols, pair<int, int> start
    ) {
        vector<vector<char>> map = map::generate_empty_maze(rows, cols);

        unordered_set<string> visited;
        map::dfs(start.first, start.second, map, visited);

        unsigned int max_dist;
        pair<int, int> end_cell;
        tie(end_cell, max_dist) = find_farthest_point(map, 1, 1);

        map[end_cell.first][end_cell.second] = TILE_GOAL;

        return {map, end_cell, max_dist};
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
    const unordered_set<char> solids = {map::TILE_WALL};

    enum Key {
        ArrowPrefix = 224,
        CtrlC = 3,
        Up = 72,
        Down = 80,
        Left = 75,
        Right = 77
    };

    const pair<int, int> EXIT_CODE = {2, 2};

    vector<vector<char>> update_matrix(
            const vector<vector<char>>& map,
            vector<vector<char>>& old_matrix,
            pair<int, int>& player_location,
            const pair<int, int>& offset
    ) {
        int x = player_location.first;
        int y = player_location.second;

        int new_x = x + offset.first;
        int new_y = y + offset.second;

        if (new_x < 0 || new_y < 0) return old_matrix;
        if (new_x > old_matrix.size() - 1 || new_y > old_matrix[0].size() - 1) return old_matrix;

        if (!solids.contains(old_matrix[new_x][new_y])) {
            vector<vector<char>> new_matrix = old_matrix;

            new_matrix[x][y] = map[x][y];
            new_matrix[new_x][new_y] = map::TILE_PLAYER;

            player_location = {new_x, new_y};

            return new_matrix;
        }

        return old_matrix;
    }

    pair<int, int> input() {
        int ch = _getch();
        if (ch == ArrowPrefix) {
            pair<int, int> offset = {0, 0};

            switch (_getch()) {
                case Up:
                    offset.first -= 1;
                    break;
                case Down:
                    offset.first += 1;
                    break;
                case Left:
                    offset.second -= 1;
                    break;
                case Right:
                    offset.second += 1;
                    break;
            }

            return offset;
        } else if (ch == CtrlC) {
            return EXIT_CODE;
        }

        return {0, 0};
    }
}

int main() {
    terminal::init();

    constexpr unsigned int rows = 35;
    constexpr unsigned int cols = 35;

    constexpr pair<int, int> start_position = {1, 1};

    vector<vector<char>> map;
    pair<int, int> end_cell;
    int max_dist;
    tie(map, end_cell, max_dist) = map::generate_maze(rows, cols, start_position);

    vector<vector<char>> old_matrix = map;

    pair<int, int> player_location = start_position;
    old_matrix[player_location.first][player_location.second] = map::TILE_PLAYER;

    render::render(old_matrix, old_matrix, true);

    vector<vector<char>> new_matrix;
    pair<int, int> offset;

    int moves = 0;

    chrono::time_point start = chrono::high_resolution_clock::now();

    while (true) {
        offset = game::input();
        new_matrix = game::update_matrix(map, old_matrix, player_location, offset);

        if (offset == pair<int, int>{2, 2}) break;

        if (new_matrix != old_matrix) {
            render::render(old_matrix, new_matrix, false);
            old_matrix = new_matrix;

            moves++;
        }

        if (player_location == end_cell) {
            chrono::time_point end = chrono::high_resolution_clock::now();
            chrono::duration duration = end - start;
            int64_t total_seconds = chrono::duration_cast<chrono::seconds>(duration).count();

            int64_t minutes = (total_seconds % 3600) / 60;
            int64_t seconds = total_seconds % 60;

            int moves_per_second = static_cast<int>(round(moves / total_seconds));

            cout << "You finished!\n"
                 << "=====================\n"
                 << "Total Time  : " << minutes << "m " << seconds << "s\n"
                 << "Total Moves : " << moves << "\n"
                 << "Min. Moves  : " << max_dist << "\n"
                 << "Moves / s   : " << moves_per_second << "\n"
                 << "=====================\n";

            break;
        }
    }

    cout << "Press enter to exit" << endl;

    terminal::deinit();

    cin.get();

    return 0;
}