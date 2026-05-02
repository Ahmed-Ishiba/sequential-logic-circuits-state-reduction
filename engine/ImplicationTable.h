#pragma once
#include "StateTypes.h"
#include <vector>
#include <set>

class ImplicationTable {
public:
    explicit ImplicationTable(int n);
    void init(const std::vector<State>& states);
    void reduce(const std::vector<State>& states);
    std::vector<std::set<char>> getClasses(const std::vector<State>& states) const;
    ReducedMachine buildReduced(const std::vector<State>& states) const;
    const Cell& get(int i, int j) const;

private:
    Cell& getMut(int i, int j);
    int  labelToIndex(const std::vector<State>& states, char lbl) const;
    std::vector<Cell> m_data;
    int m_size;
};
