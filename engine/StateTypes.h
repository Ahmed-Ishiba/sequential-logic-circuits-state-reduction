#pragma once
#include <vector>
#include <array>
#include <string>
#include <utility>

enum class CellStatus { Compatible, Incompatible, Pending };

struct Cell {
    CellStatus status = CellStatus::Pending;
    std::vector<std::pair<char, char>> dependency;
};

struct State {
    char     label  = '?';
    std::array<char, 2> ns = {'?', '?'};
    int      output = -1;

    char nextState(int x) const { return ns[x]; }

    bool operator==(const State& o) const {
        return label == o.label && ns == o.ns && output == o.output;
    }
};

struct EquivClass {
    char             newLabel = '?';     
    std::vector<char> members;           
    State            representative;     
};

struct ReducedMachine {
    std::vector<EquivClass> classes;     
    std::vector<std::pair<char,char>> labelMap;
};
