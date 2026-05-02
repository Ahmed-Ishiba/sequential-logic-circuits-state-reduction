#include "ImplicationTable.h"
#include <algorithm>
#include <stdexcept>

ImplicationTable::ImplicationTable(int n) : m_size(n) {
    m_data.resize(n * (n - 1) / 2);
}

// Map (i,j) with i>j to a flat index
static int flatIdx(int i, int j) {
    if (i < j) std::swap(i, j);
    return i * (i - 1) / 2 + j;
}

const Cell& ImplicationTable::get(int i, int j) const {
    return m_data[flatIdx(i, j)];
}

Cell& ImplicationTable::getMut(int i, int j) {
    return m_data[flatIdx(i, j)];
}

int ImplicationTable::labelToIndex(const std::vector<State>& states, char lbl) const {
    for (int i = 0; i < (int)states.size(); i++)
        if (states[i].label == lbl) return i;
    return -1;
}

void ImplicationTable::init(const std::vector<State>& states) {
    for (int i = 1; i < m_size; i++) {
        for (int j = 0; j < i; j++) {
            Cell& c = getMut(i, j);
            c.status = CellStatus::Pending;
            c.dependency.clear();

            if (states[i].output != states[j].output) {
                c.status = CellStatus::Incompatible;
                continue;
            }

            for (int g = 0; g < 2; g++) {
                char ni = states[i].nextState(g);
                char nj = states[j].nextState(g);
                if (ni != nj)
                    c.dependency.push_back({ni, nj});
            }

            if (c.dependency.empty())
                c.status = CellStatus::Compatible;
        }
    }
}

void ImplicationTable::reduce(const std::vector<State>& states) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 1; i < m_size; i++) {
            for (int j = 0; j < i; j++) {
                Cell& c = getMut(i, j);
                if (c.status != CellStatus::Pending) continue;

                for (auto& dep : c.dependency) {
                    int pi = labelToIndex(states, dep.first);
                    int pj = labelToIndex(states, dep.second);
                    if (pi == -1 || pj == -1) continue;
                    if (get(pi, pj).status == CellStatus::Incompatible) {
                        c.status = CellStatus::Incompatible;
                        changed  = true;
                        break;
                    }
                }
            }
        }
    }
}

std::vector<std::set<char>> ImplicationTable::getClasses(const std::vector<State>& states) const {
    std::vector<std::set<char>> classes;

    for (int j = 0; j < m_size - 1; j++) {
        for (int i = j + 1; i < m_size; i++) {
            auto s = get(i, j).status;
            if (s != CellStatus::Compatible && s != CellStatus::Pending)
                continue;

            char a = states[j].label;
            char b = states[i].label;

            int found = -1;
            for (int k = 0; k < (int)classes.size(); k++) {
                if (classes[k].count(a) || classes[k].count(b)) {
                    found = k;
                    break;
                }
            }
            if (found == -1) {
                classes.push_back({a, b});
            } else {
                classes[found].insert(a);
                classes[found].insert(b);
            }
        }
    }
    return classes;
}

ReducedMachine ImplicationTable::buildReduced(const std::vector<State>& states) const {
    auto classes = getClasses(states);
    std::set<char> inClass;
    for (auto& g : classes)
        for (char c : g)
            inClass.insert(c);

    std::vector<char> singletons;
    for (auto& s : states)
        if (!inClass.count(s.label))
            singletons.push_back(s.label);

    ReducedMachine rm;
    auto newLabelFor = [&](char old) -> char {
        for (int k = 0; k < (int)classes.size(); k++)
            if (classes[k].count(old)) return 'A' + k;
        for (int k = 0; k < (int)singletons.size(); k++)
            if (singletons[k] == old) return 'A' + (int)classes.size() + k;
        return old;
    };

    for (auto& s : states)
        rm.labelMap.push_back({s.label, newLabelFor(s.label)});

    for (int k = 0; k < (int)classes.size(); k++) {
        EquivClass ec;
        ec.newLabel = 'A' + k;
        for (char c : classes[k]) ec.members.push_back(c);
        for (auto& s : states) {
            if (classes[k].count(s.label)) {
                ec.representative = s;
                break;
            }
        }
        rm.classes.push_back(ec);
    }

    for (int k = 0; k < (int)singletons.size(); k++) {
        EquivClass ec;
        ec.newLabel = 'A' + (int)classes.size() + k;
        ec.members  = {singletons[k]};
        for (auto& s : states)
            if (s.label == singletons[k]) { ec.representative = s; break; }
        rm.classes.push_back(ec);
    }

    for (auto& ec : rm.classes) {
        ec.representative.ns[0] = newLabelFor(ec.representative.ns[0]);
        ec.representative.ns[1] = newLabelFor(ec.representative.ns[1]);
        ec.representative.label = ec.newLabel;
    }

    return rm;
}
