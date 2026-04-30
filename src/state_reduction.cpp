#include "state_reduction.h"
#include <iostream>

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