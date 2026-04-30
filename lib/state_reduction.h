#ifndef STATE_REDUCTION_H
#include <array>
#include <vector>
#include <utility>
#include <set>

enum class CellStatus {Compatible, Incompatible, Pending};


typedef struct{
    CellStatus status = CellStatus::Pending;
    std::vector<std::pair<char, char>> dependency;
}cell;

class state{
    public:
    char PS;
    std::array<char, 2> NS;
    int out;
    state();
    void input(int index);
    char label();
    bool operator==(const state& other);
    char nextState(int x);
    void operator=(const state&other);
};

class implicationTable{
    std::vector<cell> data;
    int size;
    public:
    implicationTable(int n);
    cell& get(int i, int j);
    int labelToIndex(std::vector<state>& states, char label);
    implicationTable& init(std::vector<state>& states);
    implicationTable& reduce(std::vector<state>& states);
    std::vector<std::set<char>> getClass(std::vector<state>& states);
    char findNewLabel(char oldLabel, std::vector<std::set<char>>& classes, 
        std::vector<state>& states,
        std::vector<char>& soloStates);
    void printReducedTable(std::vector<state>& states);

};

#endif 