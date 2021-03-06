#ifndef DICE_ROLL_H
#define DICE_ROLL_H
#include <vector>

/**
 * This simple struct is meant to bundle together data on a single roll. In this
 * case a roll can be defined as x amount of reps of one single die type: 4d10 is
 * one roll, and all of its information, including 4 (or more) randomly generated
 * die values, will be stored in this struct and passed to the view for displaying. 
 */
struct DiceRoll {

private:
    std::vector<int> rolls;

public:
    int dieType = 0;
    int reps = 0;
    int origReps = 0;
    unsigned long long int sum = 0;
    
    /**
     * @return the number of reps minus the original number, or, the number of
     * extra rolls done as a result of aces.
     */
    [[nodiscard]] constexpr int getNumAces() const {
        return reps - origReps;
    }

    /**
     * Returns the int at the given index
     *
     * @param i The index to look at
     *
     * @return The roll at the i or -1 if the index is out of bounds
     */
    [[nodiscard]] int getAt(const int i) const {
        if (i < rolls.size() && i >= 0)
            return rolls.at(i);
        else
            return -1;
    }

    /**
     * Adds a roll result to the internal vector.
     *
     * @param val The roll to add.
     */
    void addRoll(const int val) {
        rolls.push_back(val);
    }
};

#endif //NCURSES_DICEROLL_H
