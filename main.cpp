#include <ncurses.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <random>
#include <iostream>

#define ENTER 10
#define DEFAULT_SLEEP 80000

using namespace std;
long delayMicroSec = DEFAULT_SLEEP;

vector<int>& parse_roll(char*);
void print_menu(int &highlight, int &choice, WINDOW *win, const char **choices, int size, short offset);
void do_rolls(vector<int> &roll_nums, int &last_y);
int get_roll(int);
void setup_input_win(WINDOW *&in_win, bool redo, char *roll);
void set_up_what_do_win(int last_y, WINDOW *&what_do);
void handle_settings(int lastY);
void display_input_win(WINDOW *in_win, char *roll);
int get_num_length(int);
bool aces = true;

int main() {
    WINDOW *what_do = nullptr, *input_win = nullptr, *main_screen = initscr(); 
    char roll[40];
    int last_y, choice, highlight;
    vector<int> roll_nums;
    bool redo, done, sett;
    done = redo = false;
    cbreak();
    while (!done) {
        setup_input_win(input_win, redo, roll);
        last_y = 11;
        do_rolls(parse_roll(roll), last_y);
        set_up_what_do_win(last_y, what_do); //good i think
        highlight = redo ? 1 : 0;
        while (true) {
            redo = sett = false; //git
            const char *choices[] = {
                    (const char*)"Roll again", (const char*)"Reroll this roll", 
                    (const char*)"Settings", (const char*)"Exit"};
            print_menu(highlight, choice, what_do, choices, 4, 2);
            if (choice == ENTER) { //pressed enter
                if (highlight == 3) { //exit
                    done = true;
                } else if (highlight == 2) { //settings
                    handle_settings(last_y);
                    sett = true;
                    refresh();
                } else if (highlight == 1) { //redo the roll
                    redo = true;
                }
                if (!sett) {
                    clear();
                    refresh();
                    display_input_win(input_win, roll);
                    wrefresh(input_win);
                    break;
                }
            }
        }
    }
    delwin(input_win);
    delwin(what_do);
    delwin(main_screen);
    endwin();
    vector<int>().swap(roll_nums); //dealloc
    return 0;
}

void setup_input_win(WINDOW *&in_win, bool redo, char *roll) {
    int x_max = getmaxx(stdscr);
    if (in_win == nullptr) {
        in_win = newwin(4, x_max - 12, 2, 5);
    }
    wclear(in_win);
    box(in_win, 0, 0);
    mvwprintw(in_win, 1, 1, "Enter your roll:");
    if (redo) {
        mvwprintw(in_win, 2, 1, roll);
    } else {
        echo();
        curs_set(1);
        mvwgetnstr(in_win, 2, 1, roll, 40);
    }
    curs_set(0);
    noecho();
}

void display_input_win(WINDOW *in_win, char *roll) {
    box(in_win, 0, 0);
    mvwprintw(in_win, 1, 1, "Enter your roll:");
    mvwprintw(in_win, 2, 1, roll);
    curs_set(0);
    noecho();
} //wrefresh does not work in here, maybe because double pointer or something?

void set_up_what_do_win(int last_y, WINDOW *&what_do) {
    if (what_do == nullptr) {
        what_do = newwin(7, 40, last_y + 4, 5);
    } else {
        mvwin(what_do, last_y + 4, 5);
    }
    keypad(what_do, true);
    box(what_do, 0, 0);
    wattron(what_do, A_BOLD);
    mvwprintw(what_do, 1, 1, "What do?");
    wattroff(what_do, A_BOLD);
    wrefresh(what_do);
}

void print_menu(int& highlight, int& choice, WINDOW* win, const char **choices, int size,
                short offset) {
    for (int i = 0; i < size; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + offset, 1, choices[i]);
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    choice = wgetch(win);
    switch (choice) {
        case KEY_UP:
            highlight--;
            highlight = (highlight == -1) ? size - 1 : highlight;
            break;
        case KEY_DOWN:
            highlight++;
            highlight = (highlight == size) ? 0 : highlight;
            break;
        default:
            break;
    }
}

void handle_settings(const int lastY) {
    int highlight, choice;
    highlight = choice = 0;
    const char *choices[] = {
            (const char *)"Set roll delay", (const char *)"Clear roll log", 
            (const char *)"Toggle aces", (const char *)"Exit settings"};
    int num_settings = 4;
    WINDOW* setWin = newwin(num_settings + 2, 40, lastY + 4, 46);
    keypad(setWin, true);
    box(setWin, 0, 0);
    while (true) {
        wclear(setWin); 
        box(setWin, 0, 0);
        print_menu(highlight, choice, setWin, choices, num_settings, 1);
        if (choice == 10) { //use pressed enter
            if (highlight == 0) { //change roll delay
                echo();
                curs_set(1);
                char newVal[10];
                wmove(setWin, 1, 1);
                wclrtoeol(setWin);
                box(setWin, 0, 0);
                mvwprintw(setWin, 1, 1, "Value (milliseconds):");
                box(setWin, 0, 0);
                mvwgetnstr(setWin, 1, 23, newVal, 10);
                if (stol(newVal) >= 0) {
                    delayMicroSec = stol(newVal) * 1000;
                } else {
                    wmove(setWin, 1, 1);
                    curs_set(0);
                    wclrtoeol(setWin);
                    wattron(setWin, A_BOLD);
                    box(setWin, 0, 0);
                    mvwprintw(setWin, 1, 1, "invalid input");
                    wattroff(setWin, A_BOLD);
                    wrefresh(setWin);
                    usleep(1500000);
                }
            } else if (highlight == 1) { //clear log
                mvwprintw(setWin, 2, 1, "Really clear log? y/[n]");
                char real[3];
                echo();
                curs_set(1);
                mvwgetnstr(setWin, 2, 26, real, 3);
                if (strcmp(real, "y") == 0 || strcmp(real, "Y") == 0) {
                    system("> rollHistory");
                }
            } else if (highlight == 2) { //toggle aces
                aces = !aces;
                wmove(setWin, 3, 1);
                wclrtoeol(setWin);
                box(setWin, 0, 0);
                mvwprintw(setWin, 3, 1, "Aces now %s", (aces) ? "on" : "off");
                box(setWin, 0, 0);
                wrefresh(setWin);
                usleep(1500000);
            } else if (highlight == num_settings - 1) { //exit settings
                break;
            }
            curs_set(0);
            noecho();
        } 
    }
    wclear(setWin);
    wrefresh(setWin);
    delwin(setWin);
}

int get_num_length(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + get_num_length(n / 10);
    }
}

void log_rolls(const string& rolls) {
    time_t now = time(nullptr);
    string date = ctime(&now);
    string command = "echo \"" + date + rolls +"\" >> rollHistory";
    system(command.c_str());
}

void do_rolls(vector<int> &roll_nums, int &last_y) {
    int sum, totalSum, origReps, dieType, reps, numLen, rollVal, i, j;
    WINDOW *die, *stats_win, *total_win;
    string total_roll;
    totalSum = 0;
    for (i = 0; i < roll_nums.size(); i += 2) {
        sum = 0;
        dieType = roll_nums.at(i + 1);
        numLen = get_num_length(dieType);
        reps = origReps = roll_nums.at(i);
        total_roll += to_string(reps) + "d" + to_string(dieType) + ": ";
        last_y = 7 + (i * 2);
        for (j = 0; j < reps; j++) {
            usleep(delayMicroSec);
            die = newwin(3, numLen + 2, last_y, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            rollVal = get_roll(dieType);
            sum += rollVal;
            total_roll += to_string(rollVal) + " ";
            mvwprintw(die, 1, 1, "%d", rollVal);
            wrefresh(die);
            delwin(die);
            if (aces && rollVal == dieType && dieType != 1) reps++;
        }
        totalSum += sum;
        total_roll += "\n";
        stats_win = newwin(2, 32, last_y, (7 + (j * (numLen + 3))));
        mvwprintw(stats_win, 0, 0, "Sum: %d | %s ace(s)", sum,
                 aces ? to_string(reps - origReps).c_str() : "No");
        mvwprintw(stats_win, 1, 0, "<-- %dd%d", reps,
                  dieType);
        wrefresh(stats_win);
        delwin(stats_win); //unfortunately these seem to have to be in windows
    }
    roll_nums.clear();
    total_win = newwin(1, 32, last_y + 3, 5);
    mvwprintw(total_win, 0, 0, "Sum of all rolls: %d", totalSum);
    wrefresh(total_win);
    delwin(total_win);
    log_rolls(total_roll);
    total_roll.erase(0);
}

vector<int>& parse_roll(char* roll) {
    static vector<int> arr;
    arr.clear();
    string s = roll;
    int num = 0;
    for (char c : s) {
        if ((c == 'd' || c == 'D') && num == 0) {
            arr.push_back(1);
        } else if (c > 47 && c < 58) {
            num = num * 10 + (c - 48);
        } else if (num > 0) {
            arr.push_back(num);
            num = 0;
        }
    }
    if (num > 0) {
        arr.push_back(num);
    }
    return arr;
}

int get_roll(int die_type) {
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> dist(1, die_type);
    return dist(mt);
}
