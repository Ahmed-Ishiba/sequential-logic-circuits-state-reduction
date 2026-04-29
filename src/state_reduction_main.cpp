#include <iostream>
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

    state() : PS(' '), NS({'?','?'}), out(-1) {}

    state(char ps, std::array<char,2> ns, int o) : PS(ps), NS(ns), out(o) {}

    void input(int index){
        std::cout << "Enter state " << index << ":" << std::endl;
        std::cout << "  Present state label: "; std::cin >> PS;
        std::cout << "  Next state (x=0): ";    std::cin >> NS[0];
        std::cout << "  Next state (x=1): ";    std::cin >> NS[1];
        std::cout << "  Output: ";              std::cin >> out;
    }

    char label(){
        return PS;
    }
    bool operator==(const state& other){
        return (this->PS == other.PS && this->NS == other.NS && this->out == other.out);
    }
    char nextState(int x){
        return NS[x];
    }
    void operator=(const state& other){
        this->PS = other.PS;
        this->NS = other.NS;
        this->out = other.out;
    }
};

class implicationTable {
    std::vector<cell> data;
    int size;

    public:

        implicationTable(int n) : size(n) {
            data.resize(n * (n - 1) / 2);
        }

        cell& get(int i, int j){
            if (i < j) std::swap(i, j);
            return data[i * (i - 1) / 2 + j];
        }

        int labelToIndex(std::vector<state>& states, char label) {
            for (int i = 0; i < (int)states.size(); i++) {
                if (states[i].label() == label) return i;
            }
            return -1;
        }

        implicationTable& init(std::vector<state>& states){
            for (int i = 1; i < size; i++){
                for (int j = 0; j < i; j++){
                    cell& c = get(i, j);
                    // check output
                    if(states[i].out != states[j].out){
                        c.status = CellStatus::Incompatible;
                        continue;
                    }
                    // check next states
                    for(int g = 0; g < 2; g++){
                        char ns_i = states[i].nextState(g);
                        char ns_j = states[j].nextState(g);
                        if (ns_i != ns_j){
                            c.dependency.push_back({ns_i, ns_j});
                        }
                    }
                    c.status = c.dependency.empty() ? CellStatus::Compatible : CellStatus::Pending;
                }
            }
            return *this;
        }

        implicationTable& reduce(std::vector<state>& states){
            bool change_made = true;
            while(change_made){
                change_made = false;
                for(int i = 1; i < size; i++){
                    for(int j = 0; j < i; j++){
                        if(get(i, j).status == CellStatus::Pending){
                            for(auto& pair: get(i, j).dependency){
                                int p_i = labelToIndex(states, pair.first);
                                int p_j = labelToIndex(states, pair.second);
                                if(get(p_i, p_j).status == CellStatus::Incompatible){
                                    get(i, j).status = CellStatus::Incompatible;
                                    change_made = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            return *this;
        }

        std::vector<std::set<char>> getClass(std::vector<state>& states){
            std::vector<std::set<char>> classes;

            for(int j = 0; j < size - 1; j++){
                for(int i = j + 1; i < size; i++){
                    if(get(i, j).status == CellStatus::Compatible ||
                       get(i, j).status == CellStatus::Pending){

                        char a = states[j].PS;
                        char b = states[i].PS;

                        int found = -1;
                        for(int k = 0; k < (int)classes.size(); k++){
                            if(classes[k].count(a) || classes[k].count(b)){
                                found = k;
                                break;
                            }
                        }

                        if(found == -1){
                            std::set<char> group;
                            group.insert(a);
                            group.insert(b);
                            classes.push_back(group);
                        } else {
                            classes[found].insert(a);
                            classes[found].insert(b);
                        }
                    }
                }
            }
            return classes;
        }

        char findNewLabel(char oldLabel, std::vector<std::set<char>>& classes,
                          std::vector<state>& states,
                          std::vector<char>& soloStates){
            for(int k = 0; k < (int)classes.size(); k++){
                if(classes[k].count(oldLabel)){
                    return 'A' + k;
                }
            }

            for(int k = 0; k < (int)soloStates.size(); k++){
                if(soloStates[k] == oldLabel){
                    return 'A' + (int)classes.size() + k;
                }
            }
            return oldLabel; 
        }

        void printReducedTable(std::vector<state>& states){
            std::vector<std::set<char>> classes = getClass(states);

            
            std::set<char> inClass;
            for(auto& group : classes)
                for(char c : group)
                    inClass.insert(c);

            
            std::vector<char> soloStates;
            for(auto& s : states)
                if(!inClass.count(s.PS))
                    soloStates.push_back(s.PS);

            
            auto getRepState = [&](std::set<char>& group) -> state& {
                for(auto& s : states)
                    if(group.count(s.PS)) return s;
                return states[0]; 
            };

            std::cout << std::endl;
            std::cout << "Equivalent Classes:" << std::endl;
            for(int k = 0; k < (int)classes.size(); k++){
                std::cout << (char)('A' + k) << ": {";
                for(char c : classes[k]) std::cout << c << " ";
                std::cout << "}" << std::endl;
            }
            for(int k = 0; k < (int)soloStates.size(); k++){
                std::cout << (char)('A' + classes.size() + k)
                          << ": {" << soloStates[k] << "}" << std::endl;
            }

            std::cout << std::endl;
            std::cout << "Reduced State Table:" << std::endl;
            std::cout << "------------------------------------" << std::endl;
            std::cout << "PS\t| NS(x=0)\t| NS(x=1)\t| Output" << std::endl;
            std::cout << "------------------------------------" << std::endl;

            // print one row per class
            for(int k = 0; k < (int)classes.size(); k++){
                state& rep = getRepState(classes[k]);
                char newPS  = 'A' + k;
                char newNS0 = findNewLabel(rep.NS[0], classes, states, soloStates);
                char newNS1 = findNewLabel(rep.NS[1], classes, states, soloStates);
                std::cout << newPS << "\t| "
                          << newNS0 << "\t\t| "
                          << newNS1 << "\t\t| "
                          << rep.out << std::endl;
            }

            for(int k = 0; k < (int)soloStates.size(); k++){
                int idx = labelToIndex(states, soloStates[k]);
                state& s = states[idx];
                char newPS  = 'A' + (int)classes.size() + k;
                char newNS0 = findNewLabel(s.NS[0], classes, states, soloStates);
                char newNS1 = findNewLabel(s.NS[1], classes, states, soloStates);
                std::cout << newPS << "\t| "
                          << newNS0 << "\t\t| "
                          << newNS1 << "\t\t| "
                          << s.out << std::endl;
            }
            std::cout << "------------------------------------" << std::endl;
        }
};

int main(){
    int n;
    std::cout << "Enter number of states: ";
    std::cin >> n;

    std::vector<state> states(n);
    for(int i = 0; i < n; i++){
        states[i].input(i + 1);
    }

    implicationTable table(n);
    table.init(states).reduce(states);
    table.printReducedTable(states);

    return 0;
}