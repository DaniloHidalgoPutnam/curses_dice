#include <ncurses.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include <iostream>
#include "../../include/DiceController.h"
#define ENTER 10
#define DEFAULT_SLEEP 80000

using namespace std;
long delayMicroSec = DEFAULT_SLEEP;

void printMenu(int &highlight, int &choice, WINDOW *win, const char **choices,
        int size, short offset);
void
printRolls(vector<DiceRoll> *allRolls, int &lastY, DiceController &controller);
void setupInputWin(WINDOW *&inWin, bool redo, char *roll);
void setUpWhatDo(int lastY, WINDOW *&whatDo);
void handleSettings(int lastY, DiceController &controller);
void displayInputWin(WINDOW *inWin, char *roll);
int getNumLength(int n);
void handleChangeRollDelay(WINDOW *setWin);
void handleClearLog(WINDOW *setWin, DiceController &controller);
void handleToggleAces(WINDOW *setWin, DiceController &controller);


int main() {
    WINDOW *whatDo = nullptr, *inputWin = nullptr, *mainScreen = initscr();
    DiceController controller;
    char roll[40];
    int lastY, choice, highlight;
    vector<int> rollNums;
    vector<DiceRoll> *allRolls;
    bool redo, done, sett;
    done = redo = false;
    cbreak();
    while (!done) {
        setupInputWin(inputWin, redo, roll); //this collects input
        allRolls = controller.getAllRolls(roll);
        lastY = 11;
        printRolls(allRolls, lastY, controller);
        setUpWhatDo(lastY, whatDo);
        highlight = redo ? 1 : 0;
        while (true) {
            redo = sett = false;
            const char *choices[] = {
                    (const char*)"Roll again", (const char*)"Reroll this roll",
                    (const char*)"Settings", (const char*)"Exit"};
            printMenu(highlight, choice, whatDo, choices, 4, 2);
            if (choice == ENTER) { //pressed enter
                if (highlight == 3) { //exit
                    done = true;
                } else if (highlight == 2) { //settings
                    handleSettings(lastY, controller);
                    sett = true;
                } else if (highlight == 1) { //redo the roll
                    redo = true;
                }
                if (!sett) {
                    clear();
                    refresh();
                    displayInputWin(inputWin, roll);
                    wrefresh(inputWin);
                    break;
                }
            }
        }
    }
    delwin(inputWin);
    delwin(whatDo);
    delwin(mainScreen);
    endwin();
    vector<int>().swap(rollNums);
    return 0;
}

void setupInputWin(WINDOW *&inWin, bool redo, char *roll) {
    int xMax = getmaxx(stdscr);
    if (inWin == nullptr) {
        inWin = newwin(4, xMax - 12, 2, 5);
    }
    wclear(inWin);
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, "Enter your roll:");
    if (redo) {
        mvwprintw(inWin, 2, 1, roll);
    } else {
        echo();
        curs_set(1);
        mvwgetnstr(inWin, 2, 1, roll, 40);
    }
    curs_set(0);
    noecho();
}

void displayInputWin(WINDOW *inWin, char *roll) {
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, "Enter your roll:");
    mvwprintw(inWin, 2, 1, roll);
    curs_set(0);
    noecho();
} //wrefresh does not work in here, maybe because double pointer or something?

void setUpWhatDo(int lastY, WINDOW *&whatDo) {
    if (whatDo == nullptr) {
        whatDo = newwin(7, 40, lastY + 4, 5);
    } else {
        mvwin(whatDo, lastY + 4, 5);
    }
    keypad(whatDo, true);
    box(whatDo, 0, 0);
    wattron(whatDo, A_BOLD);
    mvwprintw(whatDo, 1, 1, "What do?");
    wattroff(whatDo, A_BOLD);
    wrefresh(whatDo);
}

void printMenu(int& highlight, int& choice, WINDOW* win, const char **choices, int size,
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

void handleSettings(int lastY, DiceController &controller) {
    int highlight, choice;
    highlight = choice = 0;
    const char *choices[] = {
            (const char *)"Set roll delay", (const char *)"Clear roll log",
            (const char *)"Toggle aces", (const char *)"Exit settings"};
    int numSettings = 4;
    WINDOW* setWin = newwin(numSettings + 2, 40, lastY + 4, 46);
    keypad(setWin, true);
    box(setWin, 0, 0);
    while (true) {
        wclear(setWin);
        box(setWin, 0, 0);
        printMenu(highlight, choice, setWin, choices, numSettings, 1);
        if (choice == 10) { //use pressed enter
            if (highlight == 0) { //change roll delay
                handleChangeRollDelay(setWin);
            } else if (highlight == 1) { //clear log
                handleClearLog(setWin, controller);
            } else if (highlight == 2) { //toggle aces
                handleToggleAces(setWin, controller);
            } else if (highlight == numSettings - 1) { //exit settings
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

void handleChangeRollDelay(WINDOW *setWin) {
    echo();
    curs_set(1);
    char newVal[10];
    wmove(setWin, 1, 1);
    wclrtoeol(setWin);
    box(setWin, 0, 0);
    mvwprintw(setWin, 1, 1, "Value (milliseconds):");
    box(setWin, 0, 0);
    mvwgetnstr(setWin, 1, 23, newVal, 10);
    try {
        long millis = stol(newVal);
        if (millis >= 0) {
            delayMicroSec = millis * 1000;
        } else {
            throw invalid_argument("No values less than 0.");
        }
    } catch (invalid_argument &e) {
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
}

void handleClearLog(WINDOW *setWin, DiceController &controller) {
    mvwprintw(setWin, 2, 1, "Really clear log? y/[n]");
    char real[3];
    echo();
    curs_set(1);
    mvwgetnstr(setWin, 2, 26, real, 3);
    if (strcmp(real, "y") == 0 || strcmp(real, "Y") == 0) {
        controller.clearLog();
    }
}

void handleToggleAces(WINDOW *setWin, DiceController &controller) {
    controller.toggleAces();
    wmove(setWin, 3, 1);
    wclrtoeol(setWin);
    box(setWin, 0, 0);
    mvwprintw(setWin, 3, 1, "Aces now %s", (controller.isAcing()) ? "on" : "off");
    box(setWin, 0, 0);
    wrefresh(setWin);
    usleep(1500000);
}

int getNumLength(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}

void printRolls(vector<DiceRoll> *allRolls, int &lastY, DiceController &controller) {
    int sum, totalSum, origReps, dieType, reps, numLen, rollVal, j, i = 0;
    bool aces = controller.isAcing();
    WINDOW *die, *statsWin, *totalWin;
    string totalRoll;
    totalSum = 0;
    for (const DiceRoll &dr : *allRolls) {
        sum = dr.getSum();
        dieType = dr.getDieType();
        numLen = getNumLength(dieType);
        reps = dr.getReps();
        origReps = dr.getOrigReps();
        totalRoll += to_string(reps) + "d" + to_string(dieType) + ": ";
        lastY = 7 + (i * 3);
        for (j = 0; j < reps; j++) {
            usleep(delayMicroSec);
            die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            rollVal = dr.getAt(j);
            totalRoll += to_string(rollVal) + " ";
            mvwprintw(die, 1, 1, "%d", rollVal);
            wrefresh(die);
            delwin(die);
        }
        totalSum += sum;
        totalRoll += "\n";
        statsWin = newwin(2, 32, lastY, (7 + (j * (numLen + 3))));
        string acesMsg = "| " + to_string((reps - origReps)) + " ace(s)";
        mvwprintw(statsWin, 0, 0, "Sum: %d %s", sum,
                  (aces) ? acesMsg.c_str() : "");
        mvwprintw(statsWin, 1, 0, "<-- %dd%d", reps,
                  dieType);
        wrefresh(statsWin);
        acesMsg.erase(0);
        delwin(statsWin);
        i++;
    }
    delete allRolls;
    totalWin = newwin(1, 32, lastY + 3, 5);
    mvwprintw(totalWin, 0, 0, "Sum of all rolls: %d", totalSum);
    wrefresh(totalWin);
    delwin(totalWin);
    controller.logRolls(totalRoll);
    totalRoll.erase(0);
}
