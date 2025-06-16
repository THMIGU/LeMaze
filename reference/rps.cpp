/********************************************
 *  Project     : Rock, Paper, Scissors
 *  File        : rps.cpp
 *  Author      : Kai Parsons
 *  Date        : 2025-06-09
 *  Description : Game of rock, paper,
 *                scissors.
 ********************************************/

#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <random>

using namespace std;

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

void clear_lines(int lines) {
    cout << "\033[" << lines << "A\033[0J";
}

void display_menu(const vector<string>& options, size_t selected, const string& header, bool first_display) {
    if (!first_display) clear_lines(static_cast<int>(options.size()) + 1);

    cout << header << endl;

    for (int i = 0; i <= options.size() - 1; ++i) {
        if (i == selected) {
            cout << "> " << options[i] << endl;
        } else {
            cout << "  " << options[i] << endl;
        }
    }
}

size_t selection(const vector<string>& options, const string& header) {
    size_t selected = 0;
    bool first_display = true;

    while (true) {
        display_menu(options, selected, header, first_display);
        first_display = false;

        int ch = _getch();
        if (ch == 224) {
            // 72 = UP
            // 80 = DOWN
            // 75 = LEFT
            // 77 = RIGHT

            switch (_getch()) {
                case 72:
                    selected = (selected - 1 + options.size()) % options.size();
                    break;
                case 80:
                    selected = (selected + 1) % options.size();
                    break;
            }
        } else if (ch == 13) {
            break;
        } else if (ch == 3) {
            exit(0);
        }
    }

    clear_lines(static_cast<int>(options.size()) + 1);

    return selected;
}

struct Random {
private:
    random_device rd;
    mt19937 gen;
public:
    Random() : gen(rd()) {}

    int random(int min, int max) {
        uniform_int_distribution<> distr(min, max);
        return distr(gen);
    }
};

int main() {
    terminal::init();

    const vector<string> options = {"Rock", "Paper", "Scissors"};

    while (true) {
        size_t selected = selection(options, "Rock, Paper, or Scissors?:");

        Random random;
        int cp = random.random(0, 2);

        cout << "Player chose " << options[selected] << endl;
        cout << "CP chose " << options[cp] << endl << endl;

        int result = (3 + static_cast<int>(selected) - cp) % 3;

        if (result == 0)
            cout << "Tie!" << endl;
        else if (result == 1)
            cout << "Player wins!" << endl;
        else
            cout << "CP wins!" << endl;

        cout << "(Type exit to exit or press enter to play again): ";

        string input;
        getline(cin, input);
        if (input == "exit") {
            break;
        }

        clear_lines(5);
    }

    return 0;
}